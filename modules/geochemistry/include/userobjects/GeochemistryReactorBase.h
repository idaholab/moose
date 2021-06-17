//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemicalModelDefinition.h"
#include "GeochemicalSystem.h"
#include "NodalUserObject.h"

/**
 * Base class that controls the spatio-temporal solution of geochemistry reactions
 */
class GeochemistryReactorBase : public NodalUserObject
{
public:
  /// contains params that are shared with AddGeochemistrySolverAction and its children
  static InputParameters sharedParams();

  static InputParameters validParams();
  GeochemistryReactorBase(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;
  virtual void execute() override;

  /**
   * @return a reference to the equilibrium geochemical system at the given node
   * @param node_id the ID of the node
   */
  virtual const GeochemicalSystem & getGeochemicalSystem(dof_id_type node_id) const = 0;

  /**
   * @return a reference to the most recent solver output (containing iteration info, swap info,
   * residuals, etc) at the specified node_id
   * @param node_id the node ID
   */
  virtual const std::stringstream & getSolverOutput(dof_id_type node_id) const = 0;

  /**
   * @return the total number of iterations used by the most recent solve at the specified node_id
   * @param node_id the node ID
   */
  virtual unsigned getSolverIterations(dof_id_type node_id) const = 0;

  /**
   * @return the L1norm of the residual at the end of the most recent solve at the specified node_id
   * @param node_id the node ID
   */
  virtual Real getSolverResidual(dof_id_type node_id) const = 0;

  /**
   * @return the mole additions (the first num_basis of these are additions to the bulk composition
   * of the equilibrium system, the last num_kin of these are -dt*kinetic_reaction_rate, ie
   * dt*dissolution_rate) at the specified node_id
   * @param node_id the node ID
   */
  virtual const DenseVector<Real> & getMoleAdditions(dof_id_type node_id) const = 0;

  /**
   * @return the moles dumped of the given species at the specified node_id
   * @param species the name of the species
   * @param node_id the node ID
   */
  virtual Real getMolesDumped(dof_id_type node_id, const std::string & species) const = 0;

  /// returns a reference to the PertinentGeochemicalSystem used to creat the ModelGeochemicalDatabase
  const PertinentGeochemicalSystem & getPertinentGeochemicalSystem() const { return _pgs; };

protected:
  /// Number of nodes handled by this processor (will need to be made un-const when mesh adaptivity is handled)
  const unsigned _num_my_nodes;
  /// my copy of the underlying ModelGeochemicalDatabase
  ModelGeochemicalDatabase _mgd;
  /// Reference to the original PertinentGeochemicalSystem used to create the ModelGeochemicalDatabase
  const PertinentGeochemicalSystem & _pgs;
  /// number of basis species
  const unsigned _num_basis;
  /// number of equilibrium species
  const unsigned _num_eqm;
  /// Initial value of maximum ionic strength
  const Real _initial_max_ionic_str;
  /// The ionic strength calculator
  GeochemistryIonicStrength _is;
  /// The activity calculator
  GeochemistryActivityCoefficientsDebyeHuckel _gac;
  /// Maximum number of swaps allowed during a single solve
  const unsigned _max_swaps_allowed;
  /// The species swapper
  GeochemistrySpeciesSwapper _swapper;
  /// A small value of molality
  const Real _small_molality;
  /// The solver output at each node
  std::vector<std::stringstream> _solver_output;
  /// Number of iterations used by the solver at each node
  std::vector<unsigned> _tot_iter;
  /// L1norm of the solver residual at each node
  std::vector<Real> _abs_residual;
};
