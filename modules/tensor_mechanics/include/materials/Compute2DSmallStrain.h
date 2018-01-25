//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTE2DSMALLSTRAIN_H
#define COMPUTE2DSMALLSTRAIN_H

#include "ComputeSmallStrain.h"

class Compute2DSmallStrain;

template <>
InputParameters validParams<Compute2DSmallStrain>();

/**
 * Compute2DSmallStrain defines a strain tensor, assuming small strains,
 * in 2D geometries / simulations.  ComputePlaneSmallStrain acts as a
 * base class for ComputePlaneSmallStrain and ComputeAxisymmetricRZSmallStrain
 * through the computeStrainZZ method.
 */
class Compute2DSmallStrain : public ComputeSmallStrain
{
public:
  Compute2DSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual Real computeStrainZZ() = 0;
};

#endif // COMPUTE2DSMALLSTRAIN_H
