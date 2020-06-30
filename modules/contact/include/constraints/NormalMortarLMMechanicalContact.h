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

class NormalMortarLMMechanicalContact : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  NormalMortarLMMechanicalContact(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType) final;

  const MooseVariableFE<Real> * const _secondary_disp_y;
  const MooseVariableFE<Real> * const _primary_disp_y;

  bool _computing_gap_dependence;

  const ADVariableValue * _secondary_disp_y_sln;
  const ADVariableValue * _primary_disp_y_sln;

  const Real _epsilon;

  MooseEnum _ncp_type;
};
