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
 * Compute1DIncrementalStrain defines a strain increment only for incremental
 * small strains in 1D problems, handling strains in other two directions.
 * Compute1DIncrementalStrain contains virtual methods to define the displacement gradients
 * as a general nonzero value.
 */
class Compute1DIncrementalStrain : public ComputeIncrementalSmallStrain
{
public:
  Compute1DIncrementalStrain(const InputParameters & parameters);

protected:
  /// Computes the current and old deformation gradients with the assumptions for
  /// axisymmetric 1D problems, and returns the total strain increment tensor
  void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment) override;

  /// Computes the current dUy/dY; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispYY() = 0;

  /// Computes the old dUy/dY; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispYYOld() = 0;

  /// Computes the current dUz/dZ; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispZZ() = 0;

  /// Computes the old dUz/dZ; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispZZOld() = 0;
};

#endif // COMPUTE1DINCREMENTALSTRAIN_H
