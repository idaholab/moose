/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTE1DINCREMENTALSTRAIN_H
#define COMPUTE1DINCREMENTALSTRAIN_H

#include "ComputeIncrementalSmallStrain.h"

/**
 * Compute1DIncrementalStrain defines a strain increment only for
 * incremental strains in 1D geometries, handling the out of line strains.
 * Compute1DIncrementalStrain contains a virtual method to define the strain_yy and strain_zz.
 */
class Compute1DIncrementalStrain : public ComputeIncrementalSmallStrain
{
public:
  Compute1DIncrementalStrain(const InputParameters & parameters);

protected:
  /// Computes the current and old deformation gradients with the assumptions for
  /// 1D geometries, including plane strain, generalized plane strain, and axisymmetric,
  /// and returns the total strain increment tensor
  virtual void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment) override;

  /// Computes the current dUy/dY; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeDUYDY() = 0;

  /// Computes the old dUy/dY; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeDUYDYOld() = 0;

  /// Computes the current dUz/dZ; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeDUZDZ() = 0;

  /// Computes the old dUz/dZ; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeDUZDZOld() = 0;
};

#endif //COMPUTE1DINCREMENTALSTRAIN_H
