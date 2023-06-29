//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Class responsible for generating a friction factor for the
 * friction-based pressure loss terms in the form of:
 *
 * $f(Re) = C_1 Re^{C_2}$
 *
 * This is common for flows in pipes. Designed tow ork with both
 * INSFV and PINSFV friction loss kernels.
 */
class NSFVExponentialFrictionMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();
  NSFVExponentialFrictionMaterial(const InputParameters & parameters);

private:
  /// Functor for velocity component in direction x
  const Moose::Functor<ADReal> & _u;
  /// Functor for velocity component in direction y
  const Moose::Functor<ADReal> * _v;
  /// Functor for velocity component in direction z
  const Moose::Functor<ADReal> * _w;
  /// Functor for the dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// Functor for the density
  const Moose::Functor<ADReal> & _rho;
  /// Functor for the characteristic length
  const Moose::Functor<ADReal> & _characteristic_length;
  /// Constant $C_1$ in $f(Re) = C_1 Re^{C_2}$
  const Moose::Functor<ADReal> & _c1;
  /// Constant $C_2$ in $f(Re) = C_1 Re^{C_2}$
  const Moose::Functor<ADReal> & _c2;
  /// Functor for the porosity
  const Moose::Functor<ADReal> & _porosity;
  /// The name of the output friction factor functor
  const std::string _friction_factor_name;
  /// If a factor of velocity should be included or not
  const bool _include_velicity_factor;
};
