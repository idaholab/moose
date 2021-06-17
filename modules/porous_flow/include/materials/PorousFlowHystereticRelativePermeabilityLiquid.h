//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowHystereticRelativePermeabilityBase.h"

/**
 * Material to compute liquid relative permeability for 1-phase and 2-phase hysteretic models
 */
class PorousFlowHystereticRelativePermeabilityLiquid
  : public PorousFlowHystereticRelativePermeabilityBase
{
public:
  static InputParameters validParams();

  PorousFlowHystereticRelativePermeabilityLiquid(const InputParameters & parameters);

protected:
  /// Phase number of liquid phase
  const unsigned _liquid_phase;

  /// Wetting liquid relative permeability is a cubic between liquid_modification_range * (1 - _s_gr_tp0) and (1 - 0.5 * _s_gr_tp0), where _s_gr_tp0 is the Land expression resulting from the saturation at the turning point from primary drying to first-order wetting
  const Real _liquid_modification_range;

  /// Computed value of the liquid wetting relative permeability at liquid_modification_range * (1 - _s_gr_tp0)
  MaterialProperty<Real> & _kl_begin;

  /// Computed derivative of the liquid wetting relative permeability at liquid_modification_range * (1 - _s_gr_tp0)
  MaterialProperty<Real> & _klp_begin;

  /// Computed value of the liquid wetting relative permeability at 1 - 0.5 * _s_gr_tp0
  MaterialProperty<Real> & _kl_end;

  /// Computed derivative of the liquid wetting relative permeability at 1 - 0.5 * _s_gr_tp0
  MaterialProperty<Real> & _klp_end;

  virtual void computeRelPermQp() override;

  virtual void computeTurningPoint0Info(Real tp_sat) override;
};
