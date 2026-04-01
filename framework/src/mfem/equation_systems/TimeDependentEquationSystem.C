//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TimeDependentEquationSystem.h"

namespace Moose::MFEM
{
TimeDependentEquationSystem::TimeDependentEquationSystem(
    const Moose::MFEM::TimeDerivativeMap & time_derivative_map)
  : _dt(1.0), _time_derivative_map(time_derivative_map)
{
}

void
TimeDependentEquationSystem::AddKernel(std::shared_ptr<MFEMKernel> kernel)
{
  if (!_time_derivative_map.isTimeDerivative(kernel->getTrialVariableName()))
  {
    EquationSystem::AddKernel(kernel);
    return;
  }

  const auto & trial_var_name =
      _time_derivative_map.getTimeIntegralName(kernel->getTrialVariableName());
  const auto & test_var_name = kernel->getTestVariableName();
  AddEliminatedVariableNameIfMissing(trial_var_name);
  AddTestVariableNameIfMissing(test_var_name);
  // Register new td kernels map if not present for the test variable
  if (!_td_kernels_map.Has(test_var_name))
  {
    auto kernel_field_map =
        std::make_shared<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>();
    _td_kernels_map.Register(test_var_name, std::move(kernel_field_map));
  }
  // Register new td kernels map if not present for the test/trial variable pair
  if (!_td_kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<MFEMKernel>>>();
    _td_kernels_map.Get(test_var_name)->Register(trial_var_name, std::move(kernels));
  }
  _td_kernels_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(kernel));
}

void
TimeDependentEquationSystem::BuildBilinearForms()
{
  // Register bilinear forms
  for (const auto i : index_range(_test_var_names))
  {
    const auto & test_var_name = _test_var_names.at(i);

    // Apply kernels to blf
    _blfs.Register(test_var_name, std::make_shared<mfem::ParBilinearForm>(_test_pfespaces.at(i)));
    auto blf = _blfs.GetShared(test_var_name);
    blf->SetAssemblyLevel(_assembly_level);
    ApplyBoundaryBLFIntegrators<mfem::ParBilinearForm>(
        test_var_name, test_var_name, blf, _integrated_bc_map, _dt);
    ApplyDomainBLFIntegrators<mfem::ParBilinearForm>(
        test_var_name, test_var_name, blf, _kernels_map, _dt);
    // Apply dt*du/dt contributions from the operator on the trial variable
    ApplyDomainBLFIntegrators<mfem::ParBilinearForm>(
        test_var_name, test_var_name, blf, _td_kernels_map);
    // Assemble
    blf->Assemble();

    // Apply kernels to td_blf
    _td_blfs.Register(test_var_name,
                      std::make_shared<mfem::ParBilinearForm>(_test_pfespaces.at(i)));
    auto td_blf = _td_blfs.GetShared(test_var_name);
    td_blf->SetAssemblyLevel(_assembly_level);
    ApplyDomainBLFIntegrators<mfem::ParBilinearForm>(
        test_var_name, test_var_name, td_blf, _td_kernels_map);
    // Assemble
    td_blf->Assemble();
  }
}

void
TimeDependentEquationSystem::BuildMixedBilinearForms()
{
  // Register mixed bilinear forms. Note that not all combinations may
  // have a kernel.

  // Create mblf for each test/coupled variable pair with an added kernel.
  // Mixed bilinear forms with coupled variables that are not trial variables are
  // associated with contributions from eliminated variables.
  for (const auto i : index_range(_test_var_names))
  {
    const auto & test_var_name = _test_var_names.at(i);
    auto test_mblfs = std::make_shared<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>();
    for (const auto j : index_range(_coupled_var_names))
    {
      const auto & coupled_var_name = _coupled_var_names.at(j);
      auto mblf = std::make_shared<mfem::ParMixedBilinearForm>(_coupled_pfespaces.at(j),
                                                               _test_pfespaces.at(i));
      // Register MixedBilinearForm if kernels exist for it, and assemble kernels
      if (test_var_name != coupled_var_name)
      {
        // Apply all mixed kernels with this test/trial pair
        ApplyBoundaryBLFIntegrators<mfem::ParMixedBilinearForm>(
            coupled_var_name, test_var_name, mblf, _integrated_bc_map, _dt);
        ApplyDomainBLFIntegrators<mfem::ParMixedBilinearForm>(
            coupled_var_name, test_var_name, mblf, _kernels_map, _dt);
        // Apply dt*du/dt contributions from the operator on the trial variable
        ApplyDomainBLFIntegrators<mfem::ParMixedBilinearForm>(
            coupled_var_name, test_var_name, mblf, _td_kernels_map);
        if (mblf->GetDBFI()->Size() || mblf->GetBBFI()->Size())
        {
          // Assemble mixed bilinear forms
          mblf->SetAssemblyLevel(_assembly_level);
          mblf->Assemble();
          // Register mixed bilinear forms associated with a single trial variable
          // for the current test variable
          test_mblfs->Register(coupled_var_name, mblf);
        }
      }
    }
    // Register all mixed bilinear form sets associated with a single test variable
    _mblfs.Register(test_var_name, test_mblfs);
  }

  // Register mixed bilinear forms. Note that not all combinations may
  // have a kernel.

  // Create mblf for each test/trial variable pair with an added kernel
  for (const auto i : index_range(_test_var_names))
  {
    const auto & test_var_name = _test_var_names.at(i);
    auto test_td_mblfs =
        std::make_shared<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>();
    for (const auto j : index_range(_trial_var_names))
    {
      const auto & trial_var_name = _trial_var_names.at(j);
      auto td_mblf = std::make_shared<mfem::ParMixedBilinearForm>(_test_pfespaces.at(j),
                                                                  _test_pfespaces.at(i));
      // Register MixedBilinearForm if kernels exist for it, and assemble kernels
      if (test_var_name != trial_var_name)
      {
        // Apply all mixed kernels with this test/trial pair
        ApplyDomainBLFIntegrators<mfem::ParMixedBilinearForm>(
            trial_var_name, test_var_name, td_mblf, _td_kernels_map);
        // Assemble mixed bilinear form
        if (td_mblf->GetDBFI()->Size() || td_mblf->GetBBFI()->Size())
        {
          td_mblf->SetAssemblyLevel(_assembly_level);
          td_mblf->Assemble();
          // Register mixed bilinear forms associated with a single trial variable
          // for the current test variable
          test_td_mblfs->Register(trial_var_name, td_mblf);
        }
      }
    }
    // Register all mixed bilinear forms associated with a single test variable
    _td_mblfs.Register(test_var_name, test_td_mblfs);
  }
}

void
TimeDependentEquationSystem::EliminateCoupledVariables()
{
  for (const auto & test_var_name : _test_var_names)
  {
    auto & lf = *_lfs.Get(test_var_name) *= _dt;
    for (const auto & eliminated_var_name : _eliminated_var_names)
      if (eliminated_var_name == test_var_name)
      {
        // if implicit, add contribution to linear form from terms involving state
        // The AddMult method in mfem::BilinearForm is not defined for non-legacy assembly
        mfem::Vector lf_prev(lf.Size());
        auto & td_blf = *_td_blfs.Get(test_var_name);
        td_blf.Mult(*_eliminated_variables.Get(test_var_name), lf_prev);
        lf += lf_prev;
      }
      else if (_td_mblfs.Has(test_var_name) &&
               _td_mblfs.Get(test_var_name)->Has(eliminated_var_name))
      {
        auto & td_mblf = *_td_mblfs.Get(test_var_name)->Get(eliminated_var_name);
        td_mblf.AddMult(*_eliminated_variables.Get(eliminated_var_name), lf);
      }
  }
  // Eliminate contributions from other coupled variables.
  EquationSystem::EliminateCoupledVariables();
}

} // namespace Moose::MFEM

#endif
