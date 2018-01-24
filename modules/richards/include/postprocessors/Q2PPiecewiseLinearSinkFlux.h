/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef Q2PPIECEWISELINEARSINKFLUX_H
#define Q2PPIECEWISELINEARSINKFLUX_H

#include "SideIntegralPostprocessor.h"
#include "LinearInterpolation.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"

class Function;

// Forward Declarations
class Q2PPiecewiseLinearSinkFlux;

template <>
InputParameters validParams<Q2PPiecewiseLinearSinkFlux>();

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
  Q2PPiecewiseLinearSinkFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// the sink function, which is a piecewise linear function of porepressure values
  LinearInterpolation _sink_func;

  /// the multiplier function
  Function & _m_func;

  /// the porepressure variable
  const VariableValue & _pp;

  /// whether to include density*permeability_nn/viscosity in the flux
  bool _use_mobility;

  /// whether to include relative permeability in the flux
  bool _use_relperm;

  /// fluid density, optional
  const RichardsDensity * _density;

  /// fluid viscosity, optional
  Real _viscosity;

  /// fluid relative permeaility, optional
  const RichardsRelPerm * _relperm;

  /// saturation variable, optional
  const VariableValue & _sat;

  /// medium permeability
  const MaterialProperty<RealTensorValue> & _permeability;
};

#endif
