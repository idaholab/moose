/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPLANEFINITESTRAIN_H
#define COMPUTEPLANEFINITESTRAIN_H

#include "Compute2DFiniteStrain.h"

/**
 * ComputePlaneFiniteStrain defines a strain increment and rotation
 * increment for finite strains in an Axisymmetric simulation.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputePlaneFiniteStrain : public Compute2DFiniteStrain
{
public:
  ComputePlaneFiniteStrain(const InputParameters & parameters);

protected:
  virtual Real computeDeformGradZZ();
  virtual Real computeDeformGradZZold();
};

#endif //COMPUTEPLANEFINITESTRAIN_H
