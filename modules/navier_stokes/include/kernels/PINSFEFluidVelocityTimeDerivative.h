//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"
#include "SinglePhaseFluidProperties.h"

class PMFluidVelocityTimeDerivative : public TimeDerivative
{
public:
  static InputParameters validParams();

  PMFluidVelocityTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  bool _conservative_form;
  const VariableValue & _pressure;
  const VariableValue & _temperature;
  const VariableValue & _temperature_dot;
  const MaterialProperty<Real> & _rho;
  const SinglePhaseFluidProperties & _eos;
};
