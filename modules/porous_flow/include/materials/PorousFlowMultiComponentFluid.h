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

class MultiComponentFluidProperties;

/**
 * General multicomponent fluid material. Provides quadpoint density, viscosity,
 * internal energy, enthalpy and derivatives wrt pressure, temperature and mass
 * fraction for a multicompnent fluid defined in the FluidProperties module
 */
template <bool is_ad>
class PorousFlowMultiComponentFluidTempl : public PorousFlowFluidPropertiesBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowMultiComponentFluidTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Derivative of fluid density wrt mass fraction at the qps or nodes
  MaterialProperty<Real> * const _ddensity_dX;

  /// Derivative of fluid phase viscosity wrt mass fraction at the nodes or qps
  MaterialProperty<Real> * const _dviscosity_dX;

  /// Derivative of fluid internal_energy wrt mass fraction at the qps or nodes
  MaterialProperty<Real> * const _dinternal_energy_dX;

  /// Derivative of fluid enthalpy wrt mass fraction at the qps or nodes
  MaterialProperty<Real> * const _denthalpy_dX;

  /// Multicomponent fluid properties UserObject
  const MultiComponentFluidProperties & _fp;

  /// Mass fraction variable
  const GenericVariableValue<is_ad> & _X;

  usingPorousFlowFluidPropertiesMembers;
};

typedef PorousFlowMultiComponentFluidTempl<false> PorousFlowMultiComponentFluid;
typedef PorousFlowMultiComponentFluidTempl<true> ADPorousFlowMultiComponentFluid;
