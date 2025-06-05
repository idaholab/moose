//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMultiComponentFluidBase.h"

class SinglePhaseFluidProperties;
template <bool is_ad>
class PorousFlowMultiComponentGasMixtureTempl : public PorousFlowMultiComponentFluidBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowMultiComponentGasMixtureTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Fluid properties UserObject
  std::vector<const SinglePhaseFluidProperties *> _fp;

  /// fluid name vector
  const std::vector<UserObjectName> _fp_names;

  /// number of fluid component
  const unsigned int _n_components;

  /// number of mass fraction variable
  const unsigned int _x_components;

  /// mass fraction variables
  std::vector<const GenericVariableValue<is_ad> *> _x;

  usingPorousFlowFluidPropertiesMembers;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_ddensity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dviscosity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dinternal_energy_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_denthalpy_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_console;
};
typedef PorousFlowMultiComponentGasMixtureTempl<false> PorousFlowMultiComponentGasMixture;
typedef PorousFlowMultiComponentGasMixtureTempl<true> ADPorousFlowMultiComponentGasMixture;
