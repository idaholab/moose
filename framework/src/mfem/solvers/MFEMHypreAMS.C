//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreAMS.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreAMS);

namespace
{
/**
 * mfem::HypreAMS that keeps the hypre zero-conductivity options across operator updates.
 * mfem::HypreAMS::SetOperator recreates the hypre solver whenever an operator was already set,
 * copying only the options exposed by mfem::HypreAMS, so these are re-applied afterwards.
 * Outer hypre Krylov solvers call SetOperator on their preconditioner, so this happens before
 * the first solve.
 */
class InteriorNodesHypreAMS : public mfem::HypreAMS
{
public:
  using mfem::HypreAMS::HypreAMS;

  void SetInteriorNodes(mfem::HypreParVector & interior_nodes);

  void SetProjectionFrequency(unsigned int projection_frequency);

  void SetOperator(const mfem::Operator & op) override;

private:
  /// Interior-node marker owned by MFEMHypreAMS; null if not set
  mfem::HypreParVector * _interior_nodes = nullptr;
  /// Iterations between compatible-subspace projections; 0 leaves the hypre default
  unsigned int _projection_frequency = 0;
};

void
InteriorNodesHypreAMS::SetInteriorNodes(mfem::HypreParVector & interior_nodes)
{
  _interior_nodes = &interior_nodes;
  HYPRE_AMSSetInteriorNodes(*this, *_interior_nodes);
}

void
InteriorNodesHypreAMS::SetProjectionFrequency(unsigned int projection_frequency)
{
  _projection_frequency = projection_frequency;
  HYPRE_AMSSetProjectionFrequency(*this, _projection_frequency);
}

void
InteriorNodesHypreAMS::SetOperator(const mfem::Operator & op)
{
  mfem::HypreAMS::SetOperator(op);
  if (_interior_nodes)
    HYPRE_AMSSetInteriorNodes(*this, *_interior_nodes);
  if (_projection_frequency > 0)
    HYPRE_AMSSetProjectionFrequency(*this, _projection_frequency);
}
}

InputParameters
MFEMHypreAMS::validParams()
{
  InputParameters params = Moose::MFEM::LORLinearSolverBase<mfem::HypreAMS>::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.addClassDescription("Hypre auxiliary-space Maxwell solver and preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<MFEMFESpaceName>("fespace", "H(curl) FESpace to use in HypreAMS setup.");
  params.addParam<bool>("singular",
                        false,
                        "Declare that the system is singular; use when solving curl-curl problem "
                        "if mass term is zero");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addRangeCheckedParam<unsigned int>(
      "projection_frequency",
      "projection_frequency > 0",
      "Number of iterations between each projection onto the compatible H(curl) subspace. "
      "Requires 'block'. If unset, hypre projects every 5th iteration.");
  params.setDocString("block",
                      "The list of subdomains (names or ids) in which the mass term is zero. "
                      "Typically, this may represent a zero-conductivity region in the domain.");
  return params;
}

MFEMHypreAMS::MFEMHypreAMS(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::HypreAMS>(parameters),
    MFEMBlockRestrictable(
        parameters,
        *getMFEMProblem()
             .getMFEMObject<MFEMFESpace>("MFEMFESpace", getParam<MFEMFESpaceName>("fespace"))
             .getFESpace()
             ->GetParMesh()),
    _mfem_fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>("MFEMFESpace",
                                                              getParam<MFEMFESpaceName>("fespace")))
{
  if (isParamValid("projection_frequency") && !isSubdomainRestricted())
    paramError("projection_frequency",
               "The compatible-subspace projection only applies when 'block' is set.");
  // The LOR solver's discrete gradient acts on a different H1 space, whose dofs the marker built
  // in BuildInteriorNodes() does not index.
  if (_lor && isSubdomainRestricted())
    paramError("block", "Restricting to zero-mass subdomains is not supported with LOR.");
  // 'singular' drops the gradient-subspace correction over the whole domain, which the conducting
  // region outside 'block' still needs.
  if (getParam<bool>("singular") && isSubdomainRestricted())
    paramError("singular",
               "A singular problem has a zero mass term everywhere; 'block' must not be set.");

  ConstructSolver();
}

void
MFEMHypreAMS::ConstructSolver()
{
  auto solver = std::make_unique<InteriorNodesHypreAMS>(_mfem_fespace.getFESpace().get());
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreAMS::SetSolverParameters(mfem::HypreAMS & solver)
{
  if (getParam<bool>("singular"))
    solver.SetSingularProblem();
  solver.iterative_mode = getParam<bool>("use_initial_guess");
  solver.SetPrintLevel(getParam<int>("print_level"));

  // Only set together with 'block', which excludes LOR, so the solver is never an LOR solver here
  if (isParamValid("projection_frequency"))
    cast_ref<InteriorNodesHypreAMS &>(solver).SetProjectionFrequency(
        getParam<unsigned int>("projection_frequency"));
}

void
MFEMHypreAMS::UpdateEquationSystemContext()
{
  Moose::MFEM::LORLinearSolverBase<mfem::HypreAMS>::UpdateEquationSystemContext();

  // The essential boundaries are only known once the equation system has been formed, so the
  // marker is built on the first operator update. InteriorNodesHypreAMS keeps it across later ones.
  if (isSubdomainRestricted() && !_interior_nodes)
  {
    _interior_nodes = BuildInteriorNodes();
    cast_ref<InteriorNodesHypreAMS &>(GetSolver()).SetInteriorNodes(*_interior_nodes);
  }
}

std::unique_ptr<mfem::HypreParVector>
MFEMHypreAMS::BuildInteriorNodes()
{
  if (!_equation_system)
    mooseError("Restricting HypreAMS to zero-mass subdomains requires an EquationSystem.");

  const auto & edge_fespace = *_mfem_fespace.getFESpace();
  mfem::ParMesh & pmesh = *edge_fespace.GetParMesh();
  mfem::H1_FECollection vert_fec(edge_fespace.FEColl()->GetOrder(), pmesh.Dimension());
  mfem::ParFiniteElementSpace vert_fespace(&pmesh, &vert_fec);

  // hypre uses the interior-node marker to build the discrete gradient restricted to the
  // zero-mass region, and assumes its range lies in the null space of the operator A. A node
  // qualifies only if the gradient of its basis function is annihilated by A:
  //  - every element touching it must be in the zero-mass region, since a node on the interface
  //    with the conducting region carries mass there;
  //  - it must not lie on an essential boundary, since its gradient then has a tangential trace
  //    on the eliminated boundary dofs, whose rows of A are identity rows.
  // Nodes on boundaries with natural BCs remain interior. Marking a node outside the null space
  // makes the preconditioner inconsistent and the compatible-subspace projection then corrupts
  // the outer Krylov residual.
  mfem::Array<int> ess_bdr(pmesh.bdr_attributes.Size() ? pmesh.bdr_attributes.Max() : 0);
  ess_bdr = 0;
  for (const auto & var_name : _equation_system->GetTrialVarNames())
    if (_equation_system->GetGridFunction(var_name).ParFESpace() == &edge_fespace)
    {
      const auto & var_ess_bdr = _equation_system->GetEssentialBoundaryMarkers(var_name);
      for (const auto i : make_range(var_ess_bdr.Size()))
        ess_bdr[i] |= var_ess_bdr[i];
    }

  // Nonzero entries mark essential boundary dofs
  mfem::Array<int> vertex_exterior_marker;
  vert_fespace.GetEssentialVDofs(ess_bdr, vertex_exterior_marker);

  mfem::Array<int> vdofs;
  for (const auto e : make_range(pmesh.GetNE()))
    // Element attribute not in the zero-mass subdomain list => conducting; its vertices are
    // not interior to the zero-mass region.
    if (!getSubdomainMarkers()[pmesh.GetAttribute(e) - 1])
    {
      vert_fespace.GetElementVDofs(e, vdofs);
      // The local H1 GaussLobatto basis has no sign-encoded dofs, so vdofs entries index
      // vertex_exterior_marker directly.
      for (const auto i : make_range(vdofs.Size()))
        vertex_exterior_marker[vdofs[i]] = 1;
    }

  // A dof shared between processors must be marked exterior on all of them
  // if it is exterior on any of them (bitwise OR across the shared group).
  vert_fespace.Synchronize(vertex_exterior_marker);

  mfem::ParGridFunction node_marker_gf(&vert_fespace);
  for (const auto i : make_range(node_marker_gf.Size()))
    node_marker_gf(i) = vertex_exterior_marker[i] ? 0.0 : 1.0;

  mfem::Vector node_marker_tdofs;
  node_marker_gf.GetTrueDofs(node_marker_tdofs);

  // hypre copies the (2-entry, assumed-partition) dof offsets into the vector, so it stays
  // valid after vert_fespace is destroyed.
  auto interior_nodes = std::make_unique<mfem::HypreParVector>(&vert_fespace);
  interior_nodes->Set(1.0, node_marker_tdofs);
  // Make the data valid in hypre's memory space, which hypre reads directly
  interior_nodes->HypreRead();
  return interior_nodes;
}

#endif
