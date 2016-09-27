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
 * ComputePlaneFiniteStrain defines strain increment and rotation
 * increment for finite strain under 2D planar assumptions.
 */
class ComputePlaneFiniteStrain : public Compute2DFiniteStrain
{
public:
  ComputePlaneFiniteStrain(const InputParameters & parameters);

protected:
  virtual Real computeDeformGradZZ();
  virtual Real computeDeformGradZZold();

  const VariableValue & _scalar_strain_zz;
  const VariableValue & _scalar_strain_zz_old;
};

#endif //COMPUTEPLANEFINITESTRAIN_H
