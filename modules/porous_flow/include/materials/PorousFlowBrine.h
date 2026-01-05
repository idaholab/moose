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

/**
 * Fluid properties of Brine.
 * Provides density, viscosity, derivatives wrt pressure and temperature at the quadpoints or nodes
 */

class BrineFluidProperties;
class SinglePhaseFluidProperties;
template <bool is_ad>
class PorousFlowBrineTempl : public PorousFlowMultiComponentFluidBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowBrineTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Brine fluid properties UserObject
  const BrineFluidProperties * _brine_fp;

  /// Water fluid properties UserObject
  const SinglePhaseFluidProperties * _water_fp;

  /// Flag for nodal NaCl mass fraction
  const bool _is_xnacl_nodal;

  /// NaCl mass fraction at the qps or nodes
  const GenericVariableValue<is_ad> & _xnacl;

  /// Flag to denote whether NaCl mass fraction is a nonlinear variable
  const bool _is_xnacl_pfvar;

  usingPorousFlowFluidPropertiesMembers;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_ddensity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dviscosity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dinternal_energy_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_denthalpy_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_console;
};

typedef PorousFlowBrineTempl<false> PorousFlowBrine;
typedef PorousFlowBrineTempl<true> ADPorousFlowBrine;
