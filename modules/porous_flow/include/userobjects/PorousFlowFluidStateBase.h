//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "PorousFlowCapillaryPressure.h"

/// Phase state enum
enum class FluidStatePhaseEnum
{
  LIQUID,
  GAS,
  TWOPHASE
};

/// AD data structure to pass calculated thermophysical properties
struct FluidStateProperties
{
  FluidStateProperties(){};
  FluidStateProperties(unsigned int n)
    : pressure(0.0),
      temperature(0, 0),
      saturation(0.0),
      density(0.0),
      viscosity(1.0), // to guard against division by zero
      enthalpy(0.0),
      internal_energy(0.0),
      mass_fraction(n, 0.0){};

  ADReal pressure;
  ADReal temperature;
  ADReal saturation;
  ADReal density;
  ADReal viscosity;
  ADReal enthalpy;
  ADReal internal_energy;
  std::vector<ADReal> mass_fraction;
};

/**
 * Base class for fluid states for miscible multiphase flow in porous media.
 */
class PorousFlowFluidStateBase : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateBase(const InputParameters & parameters);

  void initialize() final{};
  void execute() final{};
  void finalize() final{};

  /**
   * The maximum number of phases in this model
   * @return number of phases
   */
  unsigned int numPhases() const { return _num_phases; };

  /**
   * The maximum number of components in this model
   * @return number of components
   */
  unsigned int numComponents() const { return _num_components; };

  /**
   * The index of the aqueous phase
   * @return aqueous phase number
   */
  unsigned int aqueousPhaseIndex() const { return _aqueous_phase_number; };

  /**
   * The index of the gas phase
   * @return gas phase number
   */
  unsigned int gasPhaseIndex() const { return _gas_phase_number; };

  /**
   * Name of FluidState
   */
  virtual std::string fluidStateName() const = 0;

  /**
   * Clears the contents of the FluidStateProperties data structure
   * @param[out] fsp FluidStateProperties data structure with all data initialized to 0
   */
  void clearFluidStateProperties(std::vector<FluidStateProperties> & fsp) const;

protected:
  /// Number of phases
  unsigned int _num_phases;
  /// Number of components
  unsigned int _num_components;
  /// Phase number of the aqueous phase
  const unsigned int _aqueous_phase_number;
  /// Phase number of the gas phase
  unsigned int _gas_phase_number;
  /// Universal gas constant (J/mol/K)
  const Real _R;
  /// Conversion from C to K
  const Real _T_c2k;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc;
  /// Empty FluidStateProperties object
  FluidStateProperties _empty_fsp;
};
