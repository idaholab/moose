//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEPLANESMALLSTRAIN_H
#define COMPUTEPLANESMALLSTRAIN_H

#include "Compute2DSmallStrain.h"

class ComputePlaneSmallStrain;

template <>
InputParameters validParams<ComputePlaneSmallStrain>();

/**
 * ComputePlaneSmallStrain defines small strains under generalized
 * plane strain and plane stress assumptions, where the out of plane strain
 * can be uniformly or non-uniformly zero or nonzero.
 */
class ComputePlaneSmallStrain : public Compute2DSmallStrain
{
public:
  ComputePlaneSmallStrain(const InputParameters & parameters);

protected:
  virtual Real computeStrainZZ();

private:
  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;

  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
};

#endif // COMPUTEPLANESMALLSTRAIN_H
