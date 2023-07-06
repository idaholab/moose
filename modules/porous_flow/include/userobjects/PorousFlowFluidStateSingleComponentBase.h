//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidStateBase.h"

/**
 * Base class for miscible multiphase flow classes with a single fluid component using
 * a pressure and enthalpy formulation (eg, water and steam)
 */
class PorousFlowFluidStateSingleComponentBase : public PorousFlowFluidStateBase
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateSingleComponentBase(const InputParameters & parameters);

  /**
   * The index of the aqueous fluid component
   * @return aqueous fluid component number
   */
  unsigned int aqueousComponentIndex() const { return _fluid_component; };

  /**
   * The index of the gas fluid component
   * @return gas fluid component number
   */
  unsigned int gasComponentIndex() const { return _fluid_component; };

  /**
   * Determines the complete thermophysical state of the system for a given set of
   * primary variables
   *
   * @param pressure gas phase pressure (Pa)
   * @param enthalpy fluid enthalpy (J/kg)
   * @param qp quadpoint index
   * @param[out] fsp the FluidStateProperties struct containing all properties
   */
  virtual void thermophysicalProperties(Real pressure,
                                        Real enthalpy,
                                        unsigned int qp,
                                        std::vector<FluidStateProperties> & fsp) const = 0;

  virtual void thermophysicalProperties(const ADReal & pressure,
                                        const ADReal & enthalpy,
                                        unsigned int qp,
                                        std::vector<FluidStateProperties> & fsp) const = 0;

  unsigned int getPressureIndex() const { return _pidx; };
  unsigned int getEnthalpyIndex() const { return _hidx; };

protected:
  /// Fluid component number (only one fluid component in all phases)
  const unsigned int _fluid_component;
  /// Index of derivative wrt pressure
  const unsigned int _pidx;
  /// Index of derivative wrt enthalpy
  const unsigned int _hidx;
  /// Perturbation applied to saturation temperature to move to gas/liquid phase
  const Real _dT;
};
