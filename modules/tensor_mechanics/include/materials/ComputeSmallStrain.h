/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTESMALLSTRAIN_H
#define COMPUTESMALLSTRAIN_H

#include "ComputeStrainBase.h"

class ComputeSmallStrain;

template <>
InputParameters validParams<ComputeSmallStrain>();

/**
 * ComputeSmallStrain defines a strain tensor, assuming small strains.
 */
class ComputeSmallStrain : public ComputeStrainBase
{
public:
  ComputeSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
};

#endif // COMPUTESMALLSTRAIN_H
