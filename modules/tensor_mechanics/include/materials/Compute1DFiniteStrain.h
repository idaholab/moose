//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTE1DFINITESTRAIN_H
#define COMPUTE1DFINITESTRAIN_H

#include "ComputeFiniteStrain.h"

class Compute1DFiniteStrain;

template <>
InputParameters validParams<Compute1DFiniteStrain>();

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
