//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTECOSSERATLINEARELASTICSTRESS_H
#define COMPUTECOSSERATLINEARELASTICSTRESS_H

#include "ComputeCosseratStressBase.h"

class ComputeCosseratLinearElasticStress;

template <>
InputParameters validParams<ComputeCosseratLinearElasticStress>();

/**
 * ComputeCosseratLinearElasticStress computes the Cosserat stress
 * and couple-stress following linear elasticity theory
 * It also sets the d(stress)/d(strain) and d(couple_stress)/d(curvature)
 * tensors appropriately
 */
class ComputeCosseratLinearElasticStress : public ComputeCosseratStressBase
{
public:
  ComputeCosseratLinearElasticStress(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual void computeQpStress();
};

#endif // COMPUTECOSSERATLINEARELASTICSTRESS_H
