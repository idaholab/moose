//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "EigenproblemEquationSystem.h"
#include "MFEMEigensolverBase.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{

void
EigenproblemEquationSystem::ApplyEssentialBCs()
{
  _ess_tdof_lists.resize(_trial_var_names.size());
  for (const auto i : index_range(_trial_var_names))
  {
    mfem::ParGridFunction & trial_gf = *(_var_ess_constraints.at(i));
    _global_ess_markers.SetSize(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
    _global_ess_markers = 0;
    trial_gf.Update();
    trial_gf = _gfuncs->GetRef(_trial_var_names.at(i));
    trial_gf.ParFESpace()->GetParMesh()->MarkExternalBoundaries(_global_ess_markers);
    trial_gf.ParFESpace()->GetEssentialTrueDofs(_global_ess_markers, _ess_tdof_lists.at(i));
  }
}

void
EigenproblemEquationSystem::FormEigenproblemMatrix(mfem::OperatorHandle & op)
{
  auto & test_var_name = _test_var_names.at(0);
  auto blf = _blfs.Get(test_var_name);

  blf->EliminateEssentialBCDiag(_global_ess_markers, 1.0);
  blf->Finalize();
  op.Reset(blf->ParallelAssemble());
}

void
EigenproblemEquationSystem::FormMassMatrix(mfem::OperatorHandle & op)
{
  mfem::ConstantCoefficient one(1.0);
  mfem::ParFiniteElementSpace * fespace = _test_pfespaces.at(0);
  mfem::ParBilinearForm * m = new mfem::ParBilinearForm(fespace);

  if (fespace->GetTypicalFE()->GetRangeType() == mfem::FiniteElement::SCALAR)
    m->AddDomainIntegrator(new mfem::MassIntegrator(one));
  else
    m->AddDomainIntegrator(new mfem::VectorFEMassIntegrator(one));

  m->Assemble();
  // Shift the eigenvalue corresponding to eliminated dofs to a large value
  m->EliminateEssentialBCDiag(_global_ess_markers, std::numeric_limits<mfem::real_t>::min());
  m->Finalize();
  op.Reset(m->ParallelAssemble());
}

void
EigenproblemEquationSystem::BuildEigenproblemJacobian(mfem::BlockVector & trueX,
                                                     mfem::OperatorHandle & massRHS)
{
  mooseAssert(_test_var_names.size() == 1 && (_test_var_names.size() == _trial_var_names.size()) &&
                  (_test_var_names.at(0) == _trial_var_names.at(0)),
              "Eigensolve is only supported for single-variable, square systems");

  height = trueX.Size();
  width = trueX.Size();
  ApplyEssentialBCs();
  FormEigenproblemMatrix(_jacobian);
  FormMassMatrix(massRHS);
}

void
EigenproblemEquationSystem::RecoverEigenproblemSolution(Moose::MFEM::GridFunctions & gridfunctions,
                                                       MFEMEigensolverBase * eigensolver)
{
  mfem::Array<mfem::real_t> eigenvalues;
  eigensolver->getEigenvalues(eigenvalues);

  for (int i = 0; i < eigenvalues.Size(); ++i)
  {
    auto & trial_var_name = _trial_var_names.at(0);
    gridfunctions.Get(trial_var_name + "_" + std::to_string(i))
        ->Distribute(eigensolver->getEigenvector(i));
  }
}

} // namespace Moose::MFEM

#endif
