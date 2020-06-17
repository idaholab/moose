//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

class TangentialMortarLMMechanicalContact : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  TangentialMortarLMMechanicalContact(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType) final;

  const MooseVariableFE<Real> & _secondary_disp_y;
  const MooseVariableFE<Real> & _primary_disp_y;

  const MooseVariableFE<Real> & _contact_pressure_var;

  const ADVariableValue & _contact_pressure;

  const ADVariableValue & _secondary_x_dot;
  const ADVariableValue & _primary_x_dot;
  const ADVariableValue & _secondary_y_dot;
  const ADVariableValue & _primary_y_dot;

  const Real _friction_coeff;
  const Real _epsilon;

  const MooseEnum _ncp_type;

  const Real _c;
};
