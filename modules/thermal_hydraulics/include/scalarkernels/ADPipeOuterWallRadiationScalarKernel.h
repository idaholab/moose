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
#include "ADFunctorInterface.h"

class ThermalSolidProperties;

/**
 * Outer wall radiative heat transfer kernel for the path-integrated incompressible conjugate heat
 * transfer model
 */
class ADPipeOuterWallRadiationScalarKernel : public ADScalarKernel, public ADFunctorInterface
{
  using Base = ADScalarKernel;

public:
  ADPipeOuterWallRadiationScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();
  virtual void initialSetup() override;

protected:
  virtual ADReal computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  /// Spatial argument used for every functor evaluation
  Moose::ElemPointArg functorArg() const { return {_functor_elem, _functor_point, false}; }

  /// Solid properties object for the pipe wall
  const ThermalSolidProperties & _sp;
  /// Property state integration flag
  const bool _is_implicit;
  /// Cross-sectional area of this wall layer
  const Moose::Functor<ADReal> & _area;
  /// Perimeter of this wall node exposed to the radiative environment
  const Moose::Functor<ADReal> & _perimeter;
  /// Ambient temperature functor
  const Moose::Functor<ADReal> & _T_ambient;
  /// Emissivity functor
  const Moose::Functor<ADReal> & _emissivity;
  /// View factor functor
  const Moose::Functor<ADReal> & _view_factor;
  /// Functor by which to scale the radiative heat transfer term
  const Moose::Functor<ADReal> & _scale;
  /// Stefan-Boltzmann constant
  const Real _sigma;
  /// Point at which spatially varying functors are evaluated
  const Point _functor_point;
  /// Element containing _functor_point (nullptr if outside the mesh / not on this process)
  const Elem * _functor_elem = nullptr;
};
