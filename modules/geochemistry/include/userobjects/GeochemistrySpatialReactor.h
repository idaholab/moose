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
 * Class that controls the space-dependent and time-dependent geochemistry reactions
 */
class GeochemistrySpatialReactor : public GeochemistryReactorBase
{
public:
  /// params that are shared with AddTimeDependentReactionSolverAction
  static InputParameters sharedParams();

  static InputParameters validParams();
  GeochemistrySpatialReactor(const InputParameters & parameters);
  virtual void initialize() override;

  /// the main-thread information is used to set the other-thread information in finalize()
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

  virtual void initialSetup() override;
  virtual void execute() override;
  virtual void meshChanged() override;

  virtual const GeochemicalSystem & getGeochemicalSystem(dof_id_type node_id) const override;
  virtual const std::stringstream & getSolverOutput(dof_id_type node_id) const override;
  virtual unsigned getSolverIterations(dof_id_type node_id) const override;
  virtual Real getSolverResidual(dof_id_type node_id) const override;
  virtual const DenseVector<Real> & getMoleAdditions(dof_id_type node_id) const override;
  virtual Real getMolesDumped(dof_id_type node_id, const std::string & species) const override;

protected:
  /// Initial equilibration temperature
  const Real _initial_temperature;
  /// Temperature specified by user
  const VariableValue & _temperature;
  /// Number of kinetic species
  const unsigned _num_kin;
  /// ModelGeochemicalDatabase at each node.
  std::vector<ModelGeochemicalDatabase> _mgd_at_node;
  /// GeochemicalSystem at each node
  std::vector<GeochemicalSystem> _egs_at_node;
  /// GeochemicalSystem into which the nodal GeochemicalSystem is copied to enable recovery during adaptive timestepping
  GeochemicalSystem _egs_copy;
  /// The solver
  GeochemicalSolver _solver;
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
  /// Whether the activity or activity constraint has been removed at each node
  std::vector<std::vector<bool>> _removed_fixed_activity;
  /// Names of the species with controlled activity or fugacity
  const std::vector<std::string> _controlled_activity_species_names;
  /// Number of species with controlled activity or fugacity
  const unsigned _num_controlled_activity;
  /// Activity or fugacity of the species with controlled activity or fugacity
  std::vector<const VariableValue *> _controlled_activity_species_values;
  /// Rate of mole additions
  DenseVector<Real> _mole_rates;
  /// Moles of each basis species added at each node at the current timestep, along with kinetic rates
  std::vector<DenseVector<Real>> _mole_additions;
  /// Derivative of moles_added
  std::vector<DenseMatrix<Real>> _dmole_additions;
  /// the ramp_max_ionic_strength to use during time-stepping
  const unsigned _ramp_subsequent;
  /// _my_node_number[_current_node->id()] = node number used in this object that corresponds to _current_node->id()
  std::unordered_map<dof_id_type, unsigned> _my_node_number;
  /// whether execute has been called using this thread
  std::vector<bool> _execute_done;
  /// Whether to use adaptive timestepping at the nodes
  const bool _adaptive_timestepping;
  /// minimum value of dt allowed during adpative timestepping.  This is set to a large number if _adaptive_timestepping = false so that if any solve ever fails, this class will throw a mooseException
  const Real _dt_min;
  /// value to multiply dt my in the case of a failed solve
  const Real _dt_dec;
  /// value to multiply dt my in the case of a successful solve
  const Real _dt_inc;
  /// number of threads used to execute this UserObject
  unsigned _nthreads;

  /// Build the _my_node_number map
  void buildMyNodeNumber();
};
