/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELINEARELASTICSTRESS_CUT_DT_H
#define COMPUTELINEARELASTICSTRESS_CUT_DT_H

#include "ComputeStressBase.h"

/**
 * ComputeLinearElasticStress_CUT_DT computes the stress following linear elasticity theory (small strains)
 */
class ComputeLinearElasticStress_CUT_DT : public ComputeStressBase
{
public:
  ComputeLinearElasticStress_CUT_DT(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
};

#endif //COMPUTELINEARELASTICSTRESS_CUT_DT_H
