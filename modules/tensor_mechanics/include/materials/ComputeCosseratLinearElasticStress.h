/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECOSSERATLINEARELASTICSTRESS_H
#define COMPUTECOSSERATLINEARELASTICSTRESS_H

#include "ComputeCosseratStressBase.h"

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
