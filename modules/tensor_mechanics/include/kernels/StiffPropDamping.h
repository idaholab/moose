/*
Kernel of Stiffness Proportional Damping
*/

#pragma once

#include "StressDivergenceTensors.h"

class StiffPropDamping : public StressDivergenceTensors
{
public:
  static InputParameters validParams();

  StiffPropDamping(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<RankTwoTensor> & _stress_older;
  const MaterialProperty<RankTwoTensor> & _stress;
  Real _q;
};
