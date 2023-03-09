//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialBase.h"
#include "PorousFlowDictator.h"

/**
 * Base class for fluid properties materials. All PorousFlow fluid
 * materials must override computeQpProperties()
 */
template <bool is_ad>
class PorousFlowFluidPropertiesBaseTempl : public PorousFlowMaterialBase
{
public:
  static InputParameters validParams();

  PorousFlowFluidPropertiesBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Pore pressure at the nodes or quadpoints
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _porepressure;

  /// Fluid temperature at the nodes or quadpoints
  const GenericMaterialProperty<Real, is_ad> & _temperature;

  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _t_c2k;

  /// Universal gas constant
  const Real _R;
};

typedef PorousFlowFluidPropertiesBaseTempl<false> PorousFlowFluidPropertiesBase;
typedef PorousFlowFluidPropertiesBaseTempl<true> ADPorousFlowFluidPropertiesBase;
