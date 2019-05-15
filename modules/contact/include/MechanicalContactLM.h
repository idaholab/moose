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

template <ComputeStage>
class MechanicalContactLM;

declareADValidParams(MechanicalContactLM);

template <ComputeStage compute_stage>
class MechanicalContactLM : public ADMortarConstraint<compute_stage>
{
public:
  MechanicalContactLM(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType) final;

  const MooseVariableFE<Real> * _slave_disp_y;
  const MooseVariableFE<Real> * _master_disp_y;

  bool _computing_gap_dependence;

  const ADVariableValue * _slave_disp_y_sln;
  const ADVariableValue * _master_disp_y_sln;

  const Real _epsilon;

  usingMortarConstraintMembers;
};
