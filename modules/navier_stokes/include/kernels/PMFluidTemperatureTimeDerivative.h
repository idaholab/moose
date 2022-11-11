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
#include "PTEquationOfState.h"

class PMFluidTemperatureTimeDerivative : public TimeDerivative
{
public:
  static InputParameters validParams();

  PMFluidTemperatureTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  bool _conservative_form;
  const VariableValue & _pressure;
  const VariableValue & _porosity;
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _cp;
  const PTEquationOfState & _eos;
};
