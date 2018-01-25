//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTE2DINCREMENTALSTRAIN_H
#define COMPUTE2DINCREMENTALSTRAIN_H

#include "ComputeIncrementalSmallStrain.h"

class Compute2DIncrementalStrain;

template <>
InputParameters validParams<Compute2DIncrementalStrain>();

/**
 * Compute2DIncrementalStrain defines a strain increment only for
 * incremental strains in 2D geometries, handling the out of plane strains.
 * Compute2DIncrementalStrain contains a virtual method to define the strain_zz
 * as a general nonzero value in the inherited classes ComputePlaneIncrementalStrain
 * and ComputeAxisymmetricRZIncrementalStrain.
 */
class Compute2DIncrementalStrain : public ComputeIncrementalSmallStrain
{
public:
  Compute2DIncrementalStrain(const InputParameters & parameters);

protected:
  /// Computes the current and old deformation gradients with the assumptions for
  /// 2D geometries, including plane strain, generalized plane strain, and axisymmetric,
  /// and returns the total strain increment tensor
  virtual void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment) override;

  /// Computes the current out-of-plane displacement gradient; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispZZ() = 0;

  /// Computes the old out-of-plane displacement gradient; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispZZOld() = 0;
};

#endif // COMPUTE2DINCREMENTALSTRAIN_H
