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

  virtual const GeochemicalSystem & getGeochemicalSystem(const Point & point) const override;
  virtual const std::stringstream & getSolverOutput(const Point & point) const override;
  virtual unsigned getSolverIterations(const Point & point) const override;
  virtual Real getSolverResidual(const Point & point) const override;
  virtual const GeochemicalSystem & getGeochemicalSystem(unsigned node_id) const override;
  virtual const DenseVector<Real> & getMoleAdditions(unsigned node_id) const override;
  virtual const DenseVector<Real> & getMoleAdditions(const Point & point) const override;
  virtual Real getMolesDumped(unsigned node_id, const std::string & species) const override;

protected:
  /// Temperature specified by user
  const VariableValue & _temperature;
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
  std::vector<const VariableValue *> _source_species_rates;
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
  std::vector<const VariableValue *> _controlled_activity_species_values;
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
};
