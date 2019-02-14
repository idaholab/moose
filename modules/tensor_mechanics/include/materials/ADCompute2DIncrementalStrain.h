//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTE2DINCREMENTALSTRAIN_H
#define ADCOMPUTE2DINCREMENTALSTRAIN_H

#include "ADComputeIncrementalSmallStrain.h"

#define usingCompute2DIncrementalStrainMembers                                                     \
  usingComputeIncrementalSmallStrainMembers;                                                       \
  using ADCompute2DIncrementalStrain<compute_stage>::_out_of_plane_direction;                      \
  using ADCompute2DIncrementalStrain<compute_stage>::computeOutOfPlaneGradDisp;                    \
  using ADCompute2DIncrementalStrain<compute_stage>::computeOutOfPlaneGradDispOld

template <ComputeStage>
class ADCompute2DIncrementalStrain;

declareADValidParams(ADCompute2DIncrementalStrain);

/**
 * ADCompute2DIncrementalStrain defines a strain increment only for
 * incremental strains in 2D geometries, handling the out of plane strains.
 * ADCompute2DIncrementalStrain contains a virtual method to define the out-of-plane strain
 * as a general nonzero value in the inherited classes ComputePlaneIncrementalStrain
 * and ComputeAxisymmetricRZIncrementalStrain.
 */
template <ComputeStage compute_stage>
class ADCompute2DIncrementalStrain : public ADComputeIncrementalSmallStrain<compute_stage>
{
public:
  ADCompute2DIncrementalStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void displacementIntegrityCheck() override;
  /**
   * Computes the current and old deformation gradients with the assumptions for
   * 2D geometries, including plane strain, generalized plane strain, and axisymmetric,
   * and returns the total strain increment tensor
   */
  virtual void computeTotalStrainIncrement(ADRankTwoTensor & total_strain_increment) override;

  /**
   * Computes the current out-of-plane component of the displacement gradient; as a virtual
   * function, this function is overwritten for the specific geometries defined by inheriting
   * classes
   */
  virtual ADReal computeOutOfPlaneGradDisp() = 0;

  /**
   * Computes the old out-of-plane component of the displacement gradient; as a virtual function,
   * this function is overwritten for the specific geometries defined by inheriting classes
   */
  virtual Real computeOutOfPlaneGradDispOld() = 0;

  const unsigned int _out_of_plane_direction;

  usingComputeIncrementalSmallStrainMembers;
};

#endif // ADCOMPUTE2DINCREMENTALSTRAIN_H
