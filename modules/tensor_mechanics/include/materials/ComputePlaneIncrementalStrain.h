/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPLANEINCREMENTALSTRAIN_H
#define COMPUTEPLANEINCREMENTALSTRAIN_H

#include "Compute2DIncrementalStrain.h"

/**
 * ComputePlaneIncrementalStrain defines strain increment
 * for small strains in a 2D planar simulation.
 */
class ComputePlaneIncrementalStrain : public Compute2DIncrementalStrain
{
public:
  ComputePlaneIncrementalStrain(const InputParameters & parameters);

protected:
  virtual Real computeDeformGradZZ();
  virtual Real computeDeformGradZZold();

private:
  const VariableValue & _strain_zz;
  const VariableValue & _strain_zz_old;
};

#endif //COMPUTEPLANEINCREMENTALSTRAIN_H
