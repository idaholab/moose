//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMultiComponentFluidBase.h"

class MultiComponentFluidProperties;

/**
 * General multicomponent fluid material. Provides quadpoint density, viscosity,
 * internal energy, enthalpy and derivatives wrt pressure, temperature and mass
 * fraction for a multicompnent fluid defined in the FluidProperties module
 */
template <bool is_ad>
class PorousFlowMultiComponentFluidTempl : public PorousFlowMultiComponentFluidBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowMultiComponentFluidTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Multicomponent fluid properties UserObject
  const MultiComponentFluidProperties & _fp;

  /// Flag for nodal mass fraction
  const bool _is_X_nodal;

  /// Mass fraction variable
  const GenericVariableValue<is_ad> & _X;

  usingPorousFlowFluidPropertiesMembers;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_ddensity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dviscosity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dinternal_energy_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_denthalpy_dX;
};

typedef PorousFlowMultiComponentFluidTempl<false> PorousFlowMultiComponentFluid;
typedef PorousFlowMultiComponentFluidTempl<true> ADPorousFlowMultiComponentFluid;
