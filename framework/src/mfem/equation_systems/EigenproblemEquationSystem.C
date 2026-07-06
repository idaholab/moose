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
  _ess_tdof_lists.resize(1);
  mfem::ParGridFunction & trial_gf = *(_var_ess_constraints.at(0));
  _global_ess_markers.SetSize(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
  _global_ess_markers = 0;
  trial_gf.Update();
  trial_gf = _gfuncs->GetRef(_trial_var_names.at(0));
  // Set constrained DoF values on user-declared essential boundaries and collect their markers
  ApplyEssentialBC(_trial_var_names.at(0), trial_gf, _global_ess_markers);
  trial_gf.ParFESpace()->GetEssentialTrueDofs(_global_ess_markers, _ess_tdof_lists.at(0));
}

void
EigenproblemEquationSystem::FormEigenproblemMatrix()
{
  auto & test_var_name = _test_var_names.at(0);
  auto blf = _blfs.Get(test_var_name);

  blf->EliminateEssentialBCDiag(_global_ess_markers, 1.0);
  blf->Finalize();
  _jacobian.Reset(blf->ParallelAssemble());
}

void
EigenproblemEquationSystem::FormMassMatrix()
{
  mfem::ConstantCoefficient one(1.0);
  mfem::ParFiniteElementSpace * fespace = _test_pfespaces.at(0);
  std::unique_ptr<mfem::ParBilinearForm> m = std::make_unique<mfem::ParBilinearForm>(fespace);

  if (fespace->GetTypicalFE()->GetRangeType() == mfem::FiniteElement::SCALAR)
    m->AddDomainIntegrator(new mfem::MassIntegrator(one));
  else
    m->AddDomainIntegrator(new mfem::VectorFEMassIntegrator(one));

  m->Assemble();
  // Shift the eigenvalue corresponding to eliminated dofs to a large value. The BC DoFs on the
  // stiffness matrix are set to 1 and the mass matrix BC DoFs are set to a small value eps, such
  // that the eigenvaluesd associate with these DOFs are ~1/eps.
  m->EliminateEssentialBCDiag(_global_ess_markers, std::numeric_limits<mfem::real_t>::min());
  m->Finalize();
  _mass_rhs.Reset(m->ParallelAssemble());
}

void
EigenproblemEquationSystem::BuildEigenproblemJacobian(mfem::BlockVector & trueX)
{
  mooseAssert(_test_var_names.size() == 1 && (_test_var_names.size() == _trial_var_names.size()) &&
                  (_test_var_names.at(0) == _trial_var_names.at(0)),
              "Eigensolve is only supported for single-variable, square systems");

  height = trueX.Size();
  width = trueX.Size();
  ApplyEssentialBCs();
  FormEigenproblemMatrix();
  FormMassMatrix();
}

void
EigenproblemEquationSystem::PrepareEigensolver(EigensolverBase & solver)
{
  solver.SetMassMatrix(_mass_rhs);
  solver.SetOperator(_jacobian);
}

} // namespace Moose::MFEM

#endif
