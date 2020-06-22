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

class GapHeatConductanceAuxKernel : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  GapHeatConductanceAuxKernel(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType mortar_type) final;

  const ADMaterialProperty<Real> & _secondary_gap_conductance;
  const ADMaterialProperty<Real> & _primary_gap_conductance;

  const VariableValue & _aux_variable;
  const VariableValue & _aux_variable_old;
};
