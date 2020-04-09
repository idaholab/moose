//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidStateSingleComponentBase.h"

class SinglePhaseFluidProperties;

/**
 * Specialized class for water and vapor mixture using pressure and enthalpy.
 * Note: It is important to note that this class must be used in a non-isothernal
 * setting (i.e., with both fluid and heat transport) in order for the problem
 * to be well posed.
 */
class PorousFlowWaterVapor : public PorousFlowFluidStateSingleComponentBase
{
public:
  static InputParameters validParams();

  PorousFlowWaterVapor(const InputParameters & parameters);

  virtual std::string fluidStateName() const override;

  void thermophysicalProperties(Real pressure,
                                Real enthalpy,
                                unsigned int qp,
                                FluidStatePhaseEnum & phase_state,
                                std::vector<FluidStateProperties> & fsp) const override;

protected:
  /// Fluid properties UserObject for water
  const SinglePhaseFluidProperties & _water_fp;
  /// Molar mass of water (kg/mol)
  const Real _Mh2o;
  /// Triple point pressure of water (Pa)
  const Real _p_triple;
  /// Critical pressure of water (Pa)
  const Real _p_critical;
  /// Triple point temperature of water (K)
  const Real _T_triple;
  /// Critical temperature of water (K)
  const Real _T_critical;
};
