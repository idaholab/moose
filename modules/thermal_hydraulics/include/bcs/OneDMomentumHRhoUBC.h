//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 *
 */
class OneDMomentumHRhoUBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMomentumHRhoUBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real & _rhou;
  const VariableValue & _area;
  const VariableValue & _arhoA;
  const VariableValue & _arhoEA;
  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;

  unsigned int _arhoA_var_num;
  unsigned int _arhoEA_var_num;

public:
  static InputParameters validParams();
};
