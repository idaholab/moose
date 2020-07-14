//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DragCoefficients.h"

#define usingAnisotropicDragCoefficientsMembers                                            \
  usingMaterialMembers;                                                                    \
  using AnisotropicDragCoefficients::computeDarcyPrefactor;                 \
  using AnisotropicDragCoefficients::computeForchheimerPrefactor;           \
  using AnisotropicDragCoefficients::computeDarcyCoefficient;               \
  using AnisotropicDragCoefficients::computeForchheimerCoefficient

class AnisotropicDragCoefficients;

declareADValidParams(AnisotropicDragCoefficients);

/**
 * Material providing anisotropic drag coefficients that may be unique in
 * each coordinate direction. Each of \f$C_L\f$ and\f$C_Q\f$
 * are separated into the calculation of a prefactor and a coefficient,where the
 * total drag coefficient is the multiplication of these two. This separation
 * reduces some code duplication. The coefficient may be unique in each coordinate
 * direction, while the prefactor is the same in all directions. In addition to
 * this flexibility, a multiplier may be quickly applied on each coordinate direction.
 */
class AnisotropicDragCoefficients : public DragCoefficients
{
public:
  AnisotropicDragCoefficients(const InputParameters & parameters);

protected:
  /// Compute the drag coefficient as the multiplication of the prefactor and coefficient
  virtual void computeQpProperties() override final;

  /// Compute prefactor on Darcy coefficient
  virtual ADReal computeDarcyPrefactor() = 0;

  /// Compute prefactor on Forchheimer coefficient
  virtual ADReal computeForchheimerPrefactor() = 0;

  /// Compute Darcy coefficient in the $i$-th coordinate direction
  virtual ADReal computeDarcyCoefficient(const int & i) = 0;

  /// Compute Forchheimer coefficient in the $i$-th coordinate direction
  virtual ADReal computeForchheimerCoefficient(const int & i) = 0;

  /// constant multiplies on each of the three coordinate directions
  const RealVectorValue & _multipliers;

  using DragCoefficients::_cL;
  using DragCoefficients::_cQ;
};
