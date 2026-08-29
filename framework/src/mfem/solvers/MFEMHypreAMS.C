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
      "Number of iterations between each projection of onto the compatible H(curl) subspace.");

  return params;
}

MFEMHypreAMS::MFEMHypreAMS(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::HypreAMS>(parameters),
    _mfem_fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>(
        "MFEMFESpace", getParam<MFEMFESpaceName>("fespace"))),
    _projection_frequency(getParam<unsigned int>("projection_frequency"))
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
  // HYPRE_AMSSetInteriorNodes(ams_solver,
  //                           static_cast<mfem::HYPRE_ParVector>(*interior_nodes));
  if (_projection_frequency > 0)
    HYPRE_AMSSetProjectionFrequency(ams_solver, _projection_frequency);
}

void
MFEMHypreAMS::BuildInteriorNodes()
{
  mfem::H1_FECollection vert_fec(order, dim);
  mfem::ParFiniteElementSpace vert_fespace(&pmesh, &vert_fec);

  // REPLACE WITH INPUT EXTERIOR BLOCK
  mfem::Array<int> vertex_exterior_marker(vert_fespace.GetVSize());
  vertex_exterior_marker = 0;

  mfem::Array<int> vdofs;
  for (int e = 0; e < pmesh.GetNE(); e++)
  {
    if (!interior_set.count(pmesh.GetAttribute(e)))
    {
      vert_fespace.GetElementVDofs(e, vdofs);
      for (int i = 0; i < vdofs.Size(); i++)
      {
        vertex_exterior_marker[vdofs[i]] = 1;
      }
    }
  }
  // A dof shared between processors must be marked exterior on all of them
  // if it is exterior on any of them (bitwise OR across the shared group).
  vert_fespace.Synchronize(vertex_exterior_marker);

  mfem::Vector node_marker_l(vert_fespace.GetVSize());
  for (int i = 0; i < node_marker_l.Size(); i++)
  {
    node_marker_l(i) = 1.0 - vertex_exterior_marker[i];
  }
  mfem::ParGridFunction node_marker_gf(&vert_fespace);
  node_marker_gf = node_marker_l;

  mfem::Vector node_marker_t;
  node_marker_gf.GetTrueDofs(node_marker_t);

  mfem::HypreParVector * interior_nodes = vert_fespace.NewTrueDofVector();
  interior_nodes->Set(1.0, node_marker_t);
}

#endif
