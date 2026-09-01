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
  params.addParam<unsigned int>(
      "projection_frequency",
      0,
      "Number of iterations between each projection onto the compatible H(curl) subspace.");
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
    _mfem_fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>(
        "MFEMFESpace", getParam<MFEMFESpaceName>("fespace"))),
    _projection_frequency(getParam<unsigned int>("projection_frequency")),
    _interior_nodes(isParamSetByUser("block") ? BuildInteriorNodes() : nullptr)
{
  ConstructSolver();
}

void
MFEMHypreAMS::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreAMS>(_mfem_fespace.getFESpace().get());
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

  HYPRE_Solver ams_solver = static_cast<HYPRE_Solver>(solver);
  if (_interior_nodes)
    HYPRE_AMSSetInteriorNodes(ams_solver, static_cast<HYPRE_ParVector>(*_interior_nodes));
  if (_projection_frequency > 0)
    HYPRE_AMSSetProjectionFrequency(ams_solver, _projection_frequency);
}

std::unique_ptr<mfem::HypreParVector>
MFEMHypreAMS::BuildInteriorNodes()
{
  mfem::ParMesh & pmesh = *_mfem_fespace.getFESpace()->GetParMesh();
  mfem::H1_FECollection vert_fec(_mfem_fespace.getFESpace()->GetTypicalFE()->GetOrder(),
                                 pmesh.Dimension());
  mfem::ParFiniteElementSpace vert_fespace(&pmesh, &vert_fec);

  mfem::Array<int> vertex_exterior_marker(vert_fespace.GetVSize());
  vertex_exterior_marker = 0;

  mfem::Array<int> vdofs;
  for (const auto e : make_range(pmesh.GetNE()))
    // if element attribute is not in list of interior domains, it must be exterior (conducting)
    if (getSubdomainAttributes().Find(pmesh.GetAttribute(e)) == -1)
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
    node_marker_gf(i) = 1.0 - vertex_exterior_marker[i];

  mfem::Vector node_marker_tdofs;
  node_marker_gf.GetTrueDofs(node_marker_tdofs);

  // hypre copies the (2-entry, assumed-partition) dof offsets into the vector, so it stays
  // valid after vert_fespace is destroyed.
  auto interior_nodes = std::make_unique<mfem::HypreParVector>(&vert_fespace);
  interior_nodes->Set(1.0, node_marker_tdofs);
  return interior_nodes;
}

#endif
