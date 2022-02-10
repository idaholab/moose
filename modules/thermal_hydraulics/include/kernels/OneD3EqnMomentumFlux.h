//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Momentum flux for 1-phase flow
 */
class OneD3EqnMomentumFlux : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnMomentumFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _A;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_darhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;

  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;
  const unsigned int _arhoEA_var_number;

public:
  static InputParameters validParams();
};
