/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTESMALLSTRAIN_H
#define COMPUTESMALLSTRAIN_H

#include "ComputeStrainBase.h"

/**
 * ComputeSmallStrain defines a strain tensor, assuming small strains.
 */
class ComputeSmallStrain : public ComputeStrainBase
{
public:
  ComputeSmallStrain(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeProperties();

  const MaterialProperty<RankTwoTensor> & _stress_free_strain;
};

#endif //COMPUTESMALLSTRAIN_H
