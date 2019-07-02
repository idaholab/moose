//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

template <ComputeStage>
class ADRobinBC;

declareADValidParams(ADRobinBC);

template <ComputeStage compute_stage>
class ADRobinBC : public ADIntegratedBC<compute_stage>
{
public:
  ADRobinBC(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  usingIntegratedBCMembers;
};

