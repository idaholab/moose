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

/**
 * Implements the time derivative term for fluid energy in a porous medium. This class allows for
 * variation of density with pressure and temperature and, through chain rule multiplication of
 * partial derivatives, includes the time variation of density
 */
class PINSFEFluidTemperatureTimeDerivative : public TimeDerivative
{
public:
  static InputParameters validParams();

  PINSFEFluidTemperatureTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  bool _conservative_form;
  const VariableValue & _pressure;
  const VariableValue & _pressure_dot;
  const VariableValue & _porosity;
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _cp;
  const SinglePhaseFluidProperties & _eos;
};
