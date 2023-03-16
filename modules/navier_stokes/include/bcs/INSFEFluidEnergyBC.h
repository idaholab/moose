//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidIntegratedBCBase.h"
#include "Function.h"

/**
 * An integral BC for the energy (temperature) equation
 */
class INSFEFluidEnergyBC : public INSFEFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  INSFEFluidEnergyBC(const InputParameters & parameters);
  virtual ~INSFEFluidEnergyBC() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // Material properties
  const MaterialProperty<Real> & _cp;

  bool _has_vbc;
  bool _has_Tbc;
  const Function * _v_fn;
  const Function * _T_fn;
  const MaterialProperty<Real> & _k_elem;
  bool _has_porosity_elem;
  const VariableValue & _porosity_elem;

  bool _has_Tbranch;
  const VariableValue & _T_branch;
  unsigned int _T_branch_var_number;
};
