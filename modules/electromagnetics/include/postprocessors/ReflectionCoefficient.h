//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"
#include "MooseVariableInterface.h"

/**
 *  CURRENTLY ONLY FOR 1D PLANE WAVE SOLVES. Calculate power reflection coefficient
 *  for impinging wave on a surface. Assumes that wave of form F = F_incoming +
 *  R*F_reflected
 */
class ReflectionCoefficient : public SidePostprocessor, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  ReflectionCoefficient(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  /// compute reflection coefficient
  virtual Real computeReflection();

  /// quadrature point
  unsigned int _qp;

  /// Real component of the coupled field variable
  const VariableValue & _coupled_real;

  /// Imaginary component of the coupled field variable
  const VariableValue & _coupled_imag;

  /// Wave incidence angle
  const Real _theta;

  /// Domain length
  const Real _length;

  /// Wave number
  const Real _k;

  /// Incoming field magnitude
  const Real _incoming_mag;

  /// Value of the reflection coefficient
  Real _reflection_coefficient;
};
