//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MDFluidIntegratedBCBase.h"
#include "Function.h"

/* An integral BC for the energy (temperature) equation  */
class MDFluidEnergyBC : public MDFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  MDFluidEnergyBC(const InputParameters & parameters);
  virtual ~MDFluidEnergyBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

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
