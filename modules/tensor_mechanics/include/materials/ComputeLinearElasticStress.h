//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTELINEARELASTICSTRESS_H
#define COMPUTELINEARELASTICSTRESS_H

#include "ComputeStressBase.h"

class ComputeLinearElasticStress;

template <>
InputParameters validParams<ComputeLinearElasticStress>();

/**
 * ComputeLinearElasticStress computes the stress following linear elasticity theory (small strains)
 */
class ComputeLinearElasticStress : public ComputeStressBase
{
public:
  ComputeLinearElasticStress(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
};

#endif // COMPUTELINEARELASTICSTRESS_H
