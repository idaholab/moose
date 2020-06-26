//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "LinearInterpolation.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"

class Function;

// Forward Declarations

/**
 * This postprocessor computes the fluid flux to a Q2PPiecewiseLinearSink.
 * The flux is integral_over_boundary of
 * _sink_func*_dt  (here _sink_func is a function of porepressure)
 * and if relative permeaility and saturation are given, this integrand is multiplied by _rel_perm
 * and if _m_func is entered, this integrand is multiplied by _m_func at the quad point
 * and if density and viscosity are given, this integrand is multiplied by density*knn/viscosity,
 *      where knn is n.permeability.n where n is the normal to the boundary
 */
class Q2PPiecewiseLinearSinkFlux : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  Q2PPiecewiseLinearSinkFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// the sink function, which is a piecewise linear function of porepressure values
  LinearInterpolation _sink_func;

  /// the multiplier function
  const Function & _m_func;

  /// the porepressure variable
  const VariableValue & _pp;

  /// whether to include density*permeability_nn/viscosity in the flux
  bool _use_mobility;

  /// whether to include relative permeability in the flux
  bool _use_relperm;

  /// fluid density, optional
  const RichardsDensity * const _density;

  /// fluid viscosity, optional
  Real _viscosity;

  /// fluid relative permeaility, optional
  const RichardsRelPerm * const _relperm;

  /// saturation variable, optional
  const VariableValue & _sat;

  /// medium permeability
  const MaterialProperty<RealTensorValue> & _permeability;
};
