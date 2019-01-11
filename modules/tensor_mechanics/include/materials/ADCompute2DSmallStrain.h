//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTE2DSMALLSTRAIN_H
#define ADCOMPUTE2DSMALLSTRAIN_H

#include "ADComputeSmallStrain.h"

#define usingCompute2DSmallStrainMembers                                                           \
  usingComputeSmallStrainMembers;                                                                  \
  using ADCompute2DSmallStrain<compute_stage>::_out_of_plane_direction;                            \
  using ADCompute2DSmallStrain<compute_stage>::computeOutOfPlaneStrain

template <ComputeStage>
class ADCompute2DSmallStrain;

declareADValidParams(ADCompute2DSmallStrain);

/**
 * ADCompute2DSmallStrain defines a strain tensor, assuming small strains,
 * in 2D geometries / simulations.  ComputePlaneSmallStrain acts as a
 * base class for ComputePlaneSmallStrain and ComputeAxisymmetricRZSmallStrain
 * through the computeOutOfPlaneStrain method.
 */
template <ComputeStage compute_stage>
class ADCompute2DSmallStrain : public ADComputeSmallStrain<compute_stage>
{
public:
  ADCompute2DSmallStrain(const InputParameters & parameters);

  void initialSetup() override;
  virtual void computeProperties() override;

protected:
  virtual void displacementIntegrityCheck() override;
  virtual ADReal computeOutOfPlaneStrain() = 0;

  const unsigned int _out_of_plane_direction;

  usingComputeSmallStrainMembers;
};

#endif // ADCOMPUTE2DSMALLSTRAIN_H
