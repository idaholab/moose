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
  virtual Real computeGradDispZZ();
  virtual Real computeGradDispZZOld();

  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  const VariableValue & _scalar_out_of_plane_strain_old;

  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
};

#endif // COMPUTEPLANEFINITESTRAIN_H
