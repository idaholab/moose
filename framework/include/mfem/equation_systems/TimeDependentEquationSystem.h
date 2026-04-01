//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "EquationSystem.h"

namespace Moose::MFEM
{
/**
 * Class to store weak form components for time dependent PDEs
 */
class TimeDependentEquationSystem : public EquationSystem
{
public:
  TimeDependentEquationSystem(const Moose::MFEM::TimeDerivativeMap & time_derivative_map);

  virtual void SetTimeStep(mfem::real_t & dt) { _dt = dt; };
  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel) override;

protected:
  virtual void BuildBilinearForms() override;
  virtual void BuildMixedBilinearForms() override;
  virtual void EliminateCoupledVariables() override;

  /// Timestep size
  mfem::real_t _dt;

  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>
      _td_kernels_map;
  /// Containers to store contributions to weak form of the form (F du/dt, v)
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _td_blfs;
  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>
      _td_mblfs; // named according to trial variable

  /// Map between variable names and their time derivatives
  const Moose::MFEM::TimeDerivativeMap & _time_derivative_map;

private:
  friend class TimeDependentEquationSystemProblemOperator;
};

} // namespace Moose::MFEM

#endif
