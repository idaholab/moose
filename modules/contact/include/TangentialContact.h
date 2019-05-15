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
class TangentialContact;

declareADValidParams(TangentialContact);

template <ComputeStage compute_stage>
class TangentialContact : public ADMortarConstraint<compute_stage>
{
public:
  TangentialContact(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType type) final;

  const MooseEnum _component;

  usingMortarConstraintMembers;
};
