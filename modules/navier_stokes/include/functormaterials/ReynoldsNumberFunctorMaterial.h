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
 * Class responsible for generating functor for computing the Reynolds
 * number
 *
 * $\mathrm{Re} = \frac{\rho |u| D}{\mu}$
 *
 * where $rho$, $|u|$, $\mu$ and $D$ are the fluid density, velocity magnitude,
 * dynamic viscosity and hydraulic diameter, respectively.
 */
class ReynoldsNumberFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();
  ReynoldsNumberFunctorMaterial(const InputParameters & parameters);

private:
  /// Speed (velocity magnitude) of the fluid
  const Moose::Functor<ADReal> & _speed;
  /// Functor for the dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// Functor for the density
  const Moose::Functor<ADReal> & _rho;
  /// Functor for the characteristic length
  const Moose::Functor<Real> & _characteristic_length;
};
