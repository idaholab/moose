//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreSmoother.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreSmoother);

InputParameters
MFEMHypreSmoother::validParams()
{
  InputParameters params = Moose::MFEM::LORLinearSolverBase<mfem::HypreSmoother>::validParams();
  params.addClassDescription("Hypre relaxation smoother (Jacobi, l1 Jacobi, l1 Gauss-Seidel, ...) "
                             "for MFEM equation systems, to use as a preconditioner.");
  MooseEnum types("Jacobi=0 l1Jacobi=1 l1GS=2 l1GStr=4 lumpedJacobi=5 GS=6 OPFS=10 Chebyshev=16");
  params.addParam<MooseEnum>(
      "smoother_type",
      types,
      "Relaxation method. Defaults to l1GS when hypre runs on the CPU and to l1Jacobi when it "
      "runs on a GPU.");
  params.addParam<bool>("positive_diagonal",
                        false,
                        "Whether to replace the l1 norms of the matrix rows with their absolute "
                        "values. Otherwise they take their sign from the diagonal entries.");
  return params;
}

MFEMHypreSmoother::MFEMHypreSmoother(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::HypreSmoother>(parameters)
{
  ConstructSolver();
}

void
MFEMHypreSmoother::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreSmoother>();
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreSmoother::SetSolverParameters(mfem::HypreSmoother & solver)
{
  solver.iterative_mode = getParam<bool>("use_initial_guess");
  // Only override the type when given, to keep the default hypre chooses for the device
  if (isParamValid("smoother_type"))
    solver.SetType(getParam<MooseEnum>("smoother_type").getEnum<mfem::HypreSmoother::Type>());
  solver.SetPositiveDiagonal(getParam<bool>("positive_diagonal"));
}

void
MFEMHypreSmoother::UpdateEquationSystemContext()
{
  Moose::MFEM::LORLinearSolverBase<mfem::HypreSmoother>::UpdateEquationSystemContext();
  if (_lor)
  {
    // mfem::LORSolver sets the LOR matrix on the smoother before SetSolverParameters applies the
    // options, and the smoother computes its type-dependent data (l1 norms, eigenvalue estimates)
    // in SetOperator, so the matrix is set again.
    auto & lor_solver = cast_ref<mfem::LORSolver<mfem::HypreSmoother> &>(GetSolver());
    lor_solver.GetSolver().SetOperator(*lor_solver.GetLOR().GetAssembledSystem());
  }
}

#endif
