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
 * Implements the time derivative term for a momentum component in a porous medium. This class
 * allows for variation of density with pressure and temperature and, through chain rule
 * multiplication of partial derivatives, includes the time variation of density
 */
class PINSFEFluidVelocityTimeDerivative : public TimeDerivative
{
public:
  static InputParameters validParams();

  PINSFEFluidVelocityTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  bool _conservative_form;
  const VariableValue & _pressure;
  const VariableValue & _temperature;
  const VariableValue & _temperature_dot;
  const VariableValue & _pressure_dot;
  const MaterialProperty<Real> & _rho;
  const SinglePhaseFluidProperties & _eos;
};
