//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralVariablePostprocessor.h"
#include "LinearInterpolation.h"
#include "RichardsVarNames.h"

class Function;

// Forward Declarations

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
  static InputParameters validParams();

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
  const Function & _m_func;

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
