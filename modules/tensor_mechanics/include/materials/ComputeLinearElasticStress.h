/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELINEARELASTICSTRESS_H
#define COMPUTELINEARELASTICSTRESS_H

#include "ComputeStressBase.h"

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
