/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTE1DFINITESTRAIN_H
#define COMPUTE1DFINITESTRAIN_H

#include "ComputeFiniteStrain.h"

/**
 * Compute1DFiniteStrain defines a strain increment for finite strains in 1D problems,
 * handling strains in other two directions. It contains virtual methods to define
 * the displacement gradients as a general nonzero value.
 */
class Compute1DFiniteStrain : public ComputeFiniteStrain
{
public:
  Compute1DFiniteStrain(const InputParameters & parameters);

protected:
  void computeProperties() override;

  /// Computes the current dUy/dY; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispYY() = 0;

  /// Computes the old dUy/dY; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispYYOld() = 0;

  /// Computes the current dUz/dz; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispZZ() = 0;

  /// Computes the old dUz/dz; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeGradDispZZOld() = 0;
};

#endif // COMPUTE1DFINITESTRAIN_H
