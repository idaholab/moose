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
#include "Function.h"

class SinglePhaseFluidProperties;

/**
 * Static p, T applied at the outlet in case of reversal flow
 */
class OneDMomentumStaticPressureReverseBC : public OneDIntegratedBC
{
public:
  OneDMomentumStaticPressureReverseBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _vel;
  const VariableValue & _vel_old;
  const VariableValue & _area;
  const VariableValue & _arhoA;
  const VariableValue & _temperature;

  unsigned int _arhoA_varnum;

  /// Specified pressure
  Real _p;

  const SinglePhaseFluidProperties & _spfp;

public:
  static InputParameters validParams();
};
