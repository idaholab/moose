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
 * ComputePlaneSmallStrain defines small strains under generalized
 * plane strain assumptions, where the out of plane strain can be nonzero.
 */
class ComputePlaneSmallStrain : public Compute2DSmallStrain
{
public:
  ComputePlaneSmallStrain(const InputParameters & parameters);

protected:
  virtual Real computeStrainZZ();

private:
  const VariableValue & _scalar_strain_zz;
};

#endif //COMPUTEPLANESMALLSTRAIN_H
