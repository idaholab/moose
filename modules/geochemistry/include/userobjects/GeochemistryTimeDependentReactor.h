//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemicalSolver.h"
#include "GeochemistryReactorBase.h"

/**
 * Class that controls the time-dependent (but not space-dependent) geochemistry reactions
 */
class GeochemistryTimeDependentReactor : public GeochemistryReactorBase
{
public:
  /// params that are shared with AddTimeDependentReactionSolverAction
  static InputParameters sharedParams();

  static InputParameters validParams();
  GeochemistryTimeDependentReactor(const InputParameters & parameters);
  virtual void initialize() override;

  virtual void finalize() override;

  virtual void initialSetup() override;
  virtual void execute() override;

  virtual const GeochemicalSystem & getGeochemicalSystem(dof_id_type node_id) const override;
  virtual const std::stringstream & getSolverOutput(dof_id_type node_id) const override;
  virtual unsigned getSolverIterations(dof_id_type node_id) const override;
  virtual Real getSolverResidual(dof_id_type node_id) const override;
  virtual const DenseVector<Real> & getMoleAdditions(dof_id_type node_id) const override;
  virtual Real getMolesDumped(dof_id_type node_id, const std::string & species) const override;

protected:
  /// Temperature specified by user
  const VariableValue & _temperature;
  /// Cold temperature specified by user, which is used only when mode==4
  const VariableValue & _cold_temperature;
  /// If mode=4 then temperature is ramped from _cold_temperature to _temperature in heating_increments increments
  const unsigned _heating_increments;
  /// Temperature at which the solution is required
  Real _new_temperature;
  /// Temperature at which the _egs was last made consistent
  Real _previous_temperature;
  /// The equilibrium geochemical system that holds all the molalities, activities, etc
  GeochemicalSystem _egs;
  /// The solver
  GeochemicalSolver _solver;
  /// Number of kinetic species
  const unsigned _num_kin;
  /// Defines the time at which to close the system
  const Real _close_system_at_time;
  /// Whether the system has been closed
  bool _closed_system;
  /// Names of the source species
  const std::vector<std::string> _source_species_names;
  /// Number of source species
  const unsigned _num_source_species;
  /// Rates of the source species
  const std::vector<const VariableValue *> _source_species_rates;
  /// Names of species to remove the fixed activity or fugacity constraint from
  const std::vector<std::string> _remove_fixed_activity_name;
  /// Times at which to remove the fixed activity or fugacity from the species in _remove_fixed_activity_name
  const std::vector<Real> _remove_fixed_activity_time;
  /// Number of elements in the vector _remove_fixed_activity_name;
  const unsigned _num_removed_fixed;
  /// Whether the activity or activity constraint has been removfed
  std::vector<bool> _removed_fixed_activity;
  /// Names of the species with controlled activity or fugacity
  const std::vector<std::string> _controlled_activity_species_names;
  /// Number of species with controlled activity or fugacity
  const unsigned _num_controlled_activity;
  /// Activity or fugacity of the species with controlled activity or fugacity
  const std::vector<const VariableValue *> _controlled_activity_species_values;
  /// Moles of each basis species added at the current timestep, along with kinetic rates
  DenseVector<Real> _mole_additions;
  /// Derivative of moles_added
  DenseMatrix<Real> _dmole_additions;
  /// Mode of the system (flush, flow-through, etc)
  const VariableValue & _mode;
  /// Moles of mineral removed by dump and flow-through
  std::unordered_map<std::string, Real> _minerals_dumped;
  /// the ramp_max_ionic_strength to use during time-stepping
  const unsigned _ramp_subsequent;

  /**
   * Based on _temperature[0], mole_additions, the current
   * mass of the system and the current temperature of the system, work out the resulting
   * temperature.  This is either:
   * - _temperature[0] if no mole_additions are positive (nothing is being added to the system)
   * - a weighted sum of _temperature[0] (the temperature of the reactants being added) and the
   * current system temperature, assuming the heat capacities of the inputs equal the heat
   * capacities of the existing constituents in the system
   * @param mole_additions Number of moles of each species being added to the system
   */
  Real newTemperature(const DenseVector<Real> & mole_additions) const;

  /**
   * Activate the special "dump" mode prior to solving the geochemical system:
   * - _mole_additions for basis mineral moles are set to the free mineral mole numbers (potentially
   * overwriting a user's choice)
   * - _minerals_dumped is updated
   * - the new temperature is computed
   * - the free mineral moles and bulk mole numbers in the GeochemicalSystem are updated
   * accordingly, and _mole_additions is zero
   * - all minerals are swapped out of the basis
   */
  void preSolveDump();

  /**
   * Activate the special "flush" mode prior to solving the geochemical system:
   * - the new temperature is computed
   * - the total mass of aqueous solution entering the system (ie, without free mineral moles and
   * kinetic-mineral moles) is computed
   * - the current mass of the aqueous solution is computed
   * - _mole_additions is updated by removing species from the current aqueous solution so that the
   * final mass (after _mole_additions has been added) will be the same as the current mass of
   * aqueous solution
   */
  void preSolveFlush();

  /**
   * Activate the special "flow-through" mode after solving the geochemical system:
   * - precipitated minerals are removed by setting the free mole number of minerals (basis and
   * kinetic) is set to a small number
   * - _minerals_dumped is updated accordingly
   */
  void postSolveFlowThrough();

  /**
   * Alter _mole_additions so that it will represent the situation in which all current species are
   * removed and replaced with source_species
   */
  void removeCurrentSpecies();

  /**
   * This is relevant for mode=4 simulations (heat-exchanger simulations).  Any precipitates are
   * removed from the system, the system is then heated to temperature and re-solved, and any
   * precipitates are removed from the system
   */
  void postSolveExchanger();
};
