//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStrainBase.h"

template <ComputeStage>
class ADComputeGreenLagrangeStrain;

declareADValidParams(ADComputeGreenLagrangeStrain);

/**
 * ADComputeGreenLagrangeStrain defines a non-linear Green-Lagrange strain tensor
 */
template <ComputeStage compute_stage>
class ADComputeGreenLagrangeStrain : public ADComputeStrainBase<compute_stage>
{
public:
  ADComputeGreenLagrangeStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  usingComputeStrainBaseMembers;
};

