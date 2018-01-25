//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEPLANEFINITESTRAIN_H
#define COMPUTEPLANEFINITESTRAIN_H

#include "Compute2DFiniteStrain.h"

class ComputePlaneFiniteStrain;

template <>
InputParameters validParams<ComputePlaneFiniteStrain>();

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
