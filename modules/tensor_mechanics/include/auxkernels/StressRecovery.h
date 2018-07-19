/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef StressRecovery_H
#define StressRecovery_H

#include "NodalPatchRecovery.h"
#include "RankTwoTensor.h"

class StressRecovery;

template <>
InputParameters validParams<StressRecovery>();

class StressRecovery : public NodalPatchRecovery
{
public:
  StressRecovery(const InputParameters & parameters);
  virtual ~StressRecovery() {}

protected:
  virtual Real computeValue() override;

  /**
   * index i of stress tensor, 0=x, 1=y, 2=z
   */
  const unsigned _index_i;
  /**
   * index j of stress tensor, 0=x, 1=y, 2=z
   */
  const unsigned _index_j;
  const MaterialProperty<RankTwoTensor> & _stress;
};

#endif // StressRecovery_H
