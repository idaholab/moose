//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeIncrementalStrain.h"

/**
 * ADCompute2DIncrementalStrain defines a strain increment only for
 * incremental strains in 2D geometries, handling the out of plane strains.
 * ADCompute2DIncrementalStrain contains a virtual method to define the out-of-plane strain
 * as a general nonzero value in the inherited classes ComputePlaneIncrementalStrain
 * and ComputeAxisymmetricRZIncrementalStrain.
 */
class ADCompute2DIncrementalStrain : public ADComputeIncrementalStrain
{
public:
  static InputParameters validParams();

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
};
