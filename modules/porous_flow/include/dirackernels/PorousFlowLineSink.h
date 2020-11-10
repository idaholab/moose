//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowLineGeometry.h"
#include "PorousFlowSumQuantity.h"
#include "PorousFlowDictator.h"

/**
 * Approximates a line sink a sequence of Dirac Points
 */
class PorousFlowLineSink : public PorousFlowLineGeometry
{
public:
  static InputParameters validParams();

  PorousFlowLineSink(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Add Dirac Points to the borehole
  virtual void addPoints() override;

  /// Jacobian contribution for the derivative wrt the variable jvar
  Real jac(unsigned int jvar);

  /// If _p_or_t==0, then returns the quadpoint porepressure, else returns the quadpoint temperature
  Real ptqp() const;

  /**
   * If _p_or_t==0, then returns d(quadpoint porepressure)/d(PorousFlow variable), else returns
   * d(quadpoint temperature)/d(PorousFlow variable)
   * @param pvar The PorousFlow variable number
   */
  Real dptqp(unsigned pvar) const;

  /// Returns the flux from the line sink (before modification by mobility, etc).  Derived classes should override this
  virtual Real computeQpBaseOutflow(unsigned current_dirac_ptid) const = 0;

  /// Calculates the BaseOutflow as well as its derivative wrt jvar.  Derived classes should override this
  virtual void computeQpBaseOutflowJacobian(unsigned jvar,
                                            unsigned current_dirac_ptid,
                                            Real & outflow,
                                            Real & outflowp) const = 0;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /**
   * This is used to hold the total fluid flowing into the line sink for each time step.
   * Hence, it is positive for production wells where fluid is flowing
   * from porespace into the line sink (and hence removed from the model)
   */
  PorousFlowSumQuantity & _total_outflow_mass;

  /// Whether a quadpoint porepressure material exists (for error checking)
  const bool _has_porepressure;

  /// Whether a quadpoint temperature material exists (for error checking)
  const bool _has_temperature;

  /// Whether a mass_fraction material exists (for error checking)
  const bool _has_mass_fraction;

  /// Whether a relative permeability material exists (for error checking)
  const bool _has_relative_permeability;

  /// Whether enough materials exist to form the mobility (for error checking)
  const bool _has_mobility;

  /// Whether an enthalpy material exists (for error checking)
  const bool _has_enthalpy;

  /// Whether an internal-energy material exists (for error checking)
  const bool _has_internal_energy;

  /// whether the flux  is a function of pressure or temperature
  const enum class PorTchoice { pressure, temperature } _p_or_t;

  /// Whether the flux will be multiplied by the mass fraction
  const bool _use_mass_fraction;

  /// Whether the flux will be multiplied by the relative permeability
  const bool _use_relative_permeability;

  /// Whether the flux will be multiplied by the mobility
  const bool _use_mobility;

  /// Whether the flux will be multiplied by the enthalpy
  const bool _use_enthalpy;

  /// Whether the flux will be multiplied by the internal-energy
  const bool _use_internal_energy;

  /// The phase number
  const unsigned int _ph;

  /// The component number (only used if _use_mass_fraction==true)
  const unsigned int _sp;

  /// Quadpoint pore pressure in each phase
  const MaterialProperty<std::vector<Real>> * const _pp;

  /// d(quadpoint pore pressure in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dpp_dvar;

  /// Quadpoint temperature
  const MaterialProperty<Real> * const _temperature;

  /// d(quadpoint temperature)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _dtemperature_dvar;

  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real>> * const _fluid_density_node;

  /// d(Fluid density for each phase (at the node))/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_node_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> * const _fluid_viscosity;

  /// d(Viscosity of each component in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> * const _relative_permeability;

  /// d(Relative permeability of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _drelative_permeability_dvar;

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real>>> * const _mass_fractions;

  /// d(Mass fraction of each component in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_fractions_dvar;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> * const _enthalpy;

  /// d(enthalpy of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _denthalpy_dvar;

  /// Internal_Energy of each phase
  const MaterialProperty<std::vector<Real>> * const _internal_energy;

  /// d(internal_energy of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dinternal_energy_dvar;

  /// mass flux is multiplied by this variable evaluated at quadpoints
  const VariableValue & _multiplying_var;
};
