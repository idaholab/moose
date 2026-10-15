//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"

class ThermalSolidProperties;

/**
 * Outer wall radiative heat transfer kernel for the path-integrated incompressible conjugate heat
 * transfer model
 */
class ADPipeOuterWallRadiationScalarKernel : public ADScalarKernel
{
  using Base = ADScalarKernel;

public:
  ADPipeOuterWallRadiationScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Solid properties object for the pipe wall
  const ThermalSolidProperties & _sp;
  /// Property state integration flag
  const bool _is_implicit;
  /// Cross-sectional area of this wall layer
  const Function & _area;
  /// Perimeter of this wall node exposed to the radiative environment
  const Function & _perimeter;
  /// Ambient temperature function
  const Function & _T_ambient;
  /// Emissivity function
  const Function & _emissivity;
  /// View factor function
  const Function & _view_factor;
  /// Function by which to scale the radiative heat transfer term
  const Function & _scale;
  /// Stefan-Boltzmann constant
  const Real _sigma;
};
