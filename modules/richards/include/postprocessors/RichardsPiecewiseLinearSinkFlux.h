/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSPIECEWISELINEARSINKFLUX_H
#define RICHARDSPIECEWISELINEARSINKFLUX_H

#include "SideIntegralVariablePostprocessor.h"
#include "LinearInterpolation.h"
#include "RichardsVarNames.h"

class Function;

// Forward Declarations
class RichardsPiecewiseLinearSinkFlux;

template <>
InputParameters validParams<RichardsPiecewiseLinearSinkFlux>();

/**
 * This postprocessor computes the fluid flux to a RichardsPiecewiseLinearSink.
 * The flux is integral_over_boundary of
 * _sink_func*_dt  (here _sink_func is a function of porepressure)
 * and if _use_relperm = true, this integrand is multiplied by _rel_perm
 * and if _m_func is entered, this integrand is multiplied by _m_func at the quad point
 * and if _use_mobility = true, this integrand is multiplied by density*knn/viscosity,
 *      where knn is n.permeability.n where n is the normal to the boundary
 */
class RichardsPiecewiseLinearSinkFlux : public SideIntegralVariablePostprocessor
{
public:
  RichardsPiecewiseLinearSinkFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// the sink function, which is a piecewise linear function of porepressure values
  LinearInterpolation _sink_func;

  /// whether to include density*permeability_nn/viscosity in the flux
  bool _use_mobility;

  /// whether to include relative permeability in the flux
  bool _use_relperm;

  /// the multiplier function
  Function & _m_func;

  /// holds info regarding the Richards variable names, and their values in the simulation
  const RichardsVarNames & _richards_name_UO;

  /**
   * the index into _richards_name_UO corresponding to this Postprocessor's variable
   * eg, if the richards names are 'pwater pgas poil pplasma'
   * and the variable of this Postprocessor is pgas, then _pvar=1
   */
  unsigned int _pvar;

  /// porepressure values (only the _pvar component is used)
  const MaterialProperty<std::vector<Real>> & _pp;

  /// fluid viscosity
  const MaterialProperty<std::vector<Real>> & _viscosity;

  /// medium permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// fluid relative permeability
  const MaterialProperty<std::vector<Real>> & _rel_perm;

  /// fluid density
  const MaterialProperty<std::vector<Real>> & _density;
};

#endif
