//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

class PorousFlowJoiner;

template <>
InputParameters validParams<PorousFlowJoiner>();

/**
 * Material designed to form a std::vector of property
 * and derivatives of these wrt the nonlinear variables
 * from the individual phase properties.
 *
 * Values at the quadpoint or the nodes are formed depending on _at_qps
 *
 * Properties can be viscosities, densities, thermal conductivities , etc
 * and the user specifies the property they are interested
 * in using the pf_prop string.
 *
 * Also, using d(property)/dP, d(property)/dS, etc, and
 * dP/dvar, dS/dvar, etc, the matrix of derivatives of property
 * with respect to the nonlinear Variables,  var, are computed.
 *
 * Only values at the nodes are used - not at the quadpoints
 */
class PorousFlowJoiner : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowJoiner(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Name of material property to be joined
  const std::string _pf_prop;

  /// Derivatives of porepressure variable wrt PorousFlow variables at the qps or nodes
  const MaterialProperty<std::vector<std::vector<Real>>> & _dporepressure_dvar;

  /// Derivatives of saturation variable wrt PorousFlow variables at the qps or nodes
  const MaterialProperty<std::vector<std::vector<Real>>> & _dsaturation_dvar;

  /// Derivatives of temperature variable wrt PorousFlow variables at the qps or nodes
  const MaterialProperty<std::vector<Real>> & _dtemperature_dvar;

  /// Computed property of the phase
  MaterialProperty<std::vector<Real>> & _property;

  /// d(property)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real>>> & _dproperty_dvar;

  /// Property of each phase
  std::vector<const MaterialProperty<Real> *> _phase_property;

  /// d(property of each phase)/d(pressure)
  std::vector<const MaterialProperty<Real> *> _dphase_property_dp;

  /// d(property of each phase)/d(saturation)
  std::vector<const MaterialProperty<Real> *> _dphase_property_ds;

  /// d(property of each phase)/d(temperature)
  std::vector<const MaterialProperty<Real> *> _dphase_property_dt;
};

