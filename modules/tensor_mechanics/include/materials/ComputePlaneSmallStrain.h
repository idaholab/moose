/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPLANESMALLSTRAIN_H
#define COMPUTEPLANESMALLSTRAIN_H

#include "Compute2DSmallStrain.h"

/**
 * ComputePlaneSmallStrain defines small strains under traditional
 * plane strain assumptions, where the out of plane strain is zero.
 */
class ComputePlaneSmallStrain : public Compute2DSmallStrain
{
public:
  ComputePlaneSmallStrain(const InputParameters & parameters);

protected:
  virtual Real computeStrainZZ();
};

#endif //COMPUTEPLANESMALLSTRAIN_H
