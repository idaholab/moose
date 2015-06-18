/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEAXISYMMETRICRZFINITESTRAIN_H
#define COMPUTEAXISYMMETRICRZFINITESTRAIN_H

#include "Compute2DFiniteStrain.h"

/**
 * ComputeAxisymmetricRZFiniteStrain defines a strain increment and rotation
 * increment for finite strains in an Axisymmetric simulation.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZFiniteStrain : public Compute2DFiniteStrain
{
public:
  ComputeAxisymmetricRZFiniteStrain(const std:: string & name, InputParameters parameters);

protected:
  virtual Real computeDeformGradZZ();
  virtual Real computeDeformGradZZold();
};

#endif //COMPUTEAXISYMMETRICRZFINITESTRAIN_H
