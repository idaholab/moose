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

/*
 *Class to compute the total viscosity which considers molecular and
 *mixing length model turbulent viscosity.
 */
class MixingLengthTurbulentViscosityMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  MixingLengthTurbulentViscosityMaterial(const InputParameters & parameters);

protected:
  const unsigned int _mesh_dimension;

  /// x-component velocity
  const MooseVariableFVReal & _u_vel;

  /// y-component velocity
  const MooseVariableFVReal * const _v_vel;

  /// z-component velocity
  const MooseVariableFVReal * const _w_vel;

  /// Turbulent eddy mixing length
  const MooseVariableFVReal & _mixing_len;

  /// viscosity
  const Moose::Functor<ADReal> & _mu;

  /// density
  const Moose::Functor<ADReal> & _rho;
};
