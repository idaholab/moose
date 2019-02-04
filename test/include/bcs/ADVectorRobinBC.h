//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADVECTORROBINBC_H
#define ADVECTORROBINBC_H

#include "ADVectorIntegratedBC.h"

template <ComputeStage>
class ADVectorRobinBC;

declareADValidParams(ADVectorRobinBC);

template <ComputeStage compute_stage>
class ADVectorRobinBC : public ADVectorIntegratedBC<compute_stage>
{
public:
  ADVectorRobinBC(const InputParameters & parameters);

protected:
  ADResidual computeQpResidual() override;

  usingVectorIntegratedBCMembers;
};

#endif // ADVECTORROBINBC_H
