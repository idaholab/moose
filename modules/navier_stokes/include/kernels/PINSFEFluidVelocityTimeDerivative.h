//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  bool _conservative_form;
  const VariableValue & _pressure;
  const VariableValue & _temperature;
  const VariableValue & _temperature_dot;
  const VariableValue & _pressure_dot;
  const VariableValue & _d_pressure_dot_du;
  const VariableValue & _d_temperature_dot_du;
  unsigned int _pressure_var_number;
  unsigned int _temperature_var_number;
  const MaterialProperty<Real> & _rho;
  const SinglePhaseFluidProperties & _eos;
};
