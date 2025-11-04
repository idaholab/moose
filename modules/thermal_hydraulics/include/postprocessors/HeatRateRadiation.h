//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Integrates a radiative heat flux over a boundary.
 */
class HeatRateRadiation : public SideIntegralPostprocessor
{
public:
  HeatRateRadiation(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Ambient temperature
  const Moose::Functor<Real> & _T_ambient;
  /// Emissivity
  const Moose::Functor<Real> & _emissivity;
  /// View factor
  const Moose::Functor<Real> & _view_factor;
  /// Stefan-Boltzmann constant
  const Real & _sigma_stefan_boltzmann;
  /// Functor by which to scale the heat flux
  const Moose::Functor<Real> & _scale;

public:
  static InputParameters validParams();
};
