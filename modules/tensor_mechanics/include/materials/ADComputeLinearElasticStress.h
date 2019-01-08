//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTELINEARELASTICSTRESS_H
#define ADCOMPUTELINEARELASTICSTRESS_H

#include "ADComputeStressBase.h"

template <ComputeStage>
class ADComputeLinearElasticStress;

declareADValidParams(ADComputeLinearElasticStress);

/**
 * ADComputeLinearElasticStress computes the stress following linear elasticity theory (small
 * strains)
 */
template <ComputeStage compute_stage>
class ADComputeLinearElasticStress : public ADComputeStressBase<compute_stage>
{
public:
  ADComputeLinearElasticStress(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual void computeQpStress();

  usingMaterialMembers;
  usingComputeStressBaseMembers;
};

#endif // ADCOMPUTELINEARELASTICSTRESS_H
