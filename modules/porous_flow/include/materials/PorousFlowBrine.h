//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidPropertiesBase.h"
#include "BrineFluidProperties.h"

/**
 * Fluid properties of Brine.
 * Provides density, viscosity, derivatives wrt pressure and temperature at the quadpoints or nodes
 */
class PorousFlowBrine : public PorousFlowFluidPropertiesBase
{
public:
  static InputParameters validParams();

  PorousFlowBrine(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Derivative of fluid density wrt salt mass fraction at the qps or nodes
  MaterialProperty<Real> * const _ddensity_dX;

  /// Derivative of fluid phase viscosity wrt salt mass fraction at the nodes or qps
  MaterialProperty<Real> * const _dviscosity_dX;

  /// Derivative of fluid internal_energy wrt salt mass fraction at the qps or nodes
  MaterialProperty<Real> * const _dinternal_energy_dX;

  /// Derivative of fluid enthalpy wrt salt mass fraction at the qps or nodes
  MaterialProperty<Real> * const _denthalpy_dX;

  /// Brine fluid properties UserObject
  const BrineFluidProperties * _brine_fp;

  /// Water fluid properties UserObject
  const SinglePhaseFluidProperties * _water_fp;

  /// NaCl mass fraction at the qps or nodes
  const VariableValue & _xnacl;
};
