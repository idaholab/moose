//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AnisotropicDragCoefficients.h"
#include "Function.h"

class FunctionAnisotropicDragCoefficients;

declareADValidParams(FunctionAnisotropicDragCoefficients);

/**
 * Material providing a function anisotropic drag coefficient, where \f$c_L\f$ and
 * \f$C_Q\f$ are unique in each direction.
 */
class FunctionAnisotropicDragCoefficients : public AnisotropicDragCoefficients
{
public:
  FunctionAnisotropicDragCoefficients(const InputParameters & parameters);

protected:
  virtual ADReal computeDarcyCoefficient(const int & i) override;

  virtual ADReal computeForchheimerCoefficient(const int & i) override;

  virtual ADReal computeDarcyPrefactor() override;

  virtual ADReal computeForchheimerPrefactor() override;

  /// Darcy coefficient in each coordinate direction
  std::vector<const Function *> _darcy_coefficient;

  /// Forchheimer coefficient in each coordinate direction
  std::vector<const Function *> _forchheimer_coefficient;

  /// speed
  const ADMaterialProperty<Real> & _speed;

  /// fluid density
  const ADMaterialProperty<Real> & _rho;

  /// fluid dynamic viscosity
  const ADMaterialProperty<Real> & _mu;

};
