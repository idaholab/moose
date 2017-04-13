/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTE2DSMALLSTRAIN_H
#define COMPUTE2DSMALLSTRAIN_H

#include "ComputeSmallStrain.h"

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
