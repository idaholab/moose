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
#include "GeochemistrySpeciesSwapper.h"
#include "Output.h"
#include "UserObjectInterface.h"

/**
 * Queries and performs simple manipulations on a geochemical model
 */
class GeochemicalModelInterrogator : public Output, public UserObjectInterface
{
public:
  /// params that are shared with the AddGeochemicalModelInterrogatorAction
  static InputParameters sharedParams();

  static InputParameters validParams();

  GeochemicalModelInterrogator(const InputParameters & parameters);

protected:
  virtual void output(const ExecFlagType & type) override;

  ModelGeochemicalDatabase _mgd;
  GeochemistrySpeciesSwapper _swapper;
  const std::vector<std::string> _swap_out;
  const std::vector<std::string> _swap_in;
  const unsigned _precision;
  const enum class InterrogationChoiceEnum { REACTION, ACTIVITY, EQM_TEMPERATURE } _interrogation;
  const Real _temperature;
  const std::vector<std::string> _activity_species;
  const std::vector<Real> _activity_values;
  const std::string _equilibrium_species;

private:
  /// provide a list of the equilibrium species of interest to the Interrogator
  std::vector<std::string> eqmSpeciesOfInterest() const;

  /// output nicely-formatted reaction info to console
  void outputReaction(const std::string & eqm_species) const;

  /// output activity info to console
  void outputActivity(const std::string & eqm_species) const;

  /// output temperature info to console
  void outputTemperature(const std::string & eqm_species) const;

  /// return true iff the activity is known for the species (it is a mineral or the user has set the activity)
  bool knownActivity(const std::string & species) const;

  /// return the activity for the species.  Note that knownActivity should be checked before calling getActivity.
  Real getActivity(const std::string & species) const;

  /**
   * @return The value of temperature for which log10K = rhs.  If no solution is possible, NaN is
   * returned
   * @param reference_log10k values of log10K at the values of temperature in _mgd.tmperatures. Only
   * reference_log10K(0, *) will be used.
   * rhs The right-hand side
   */
  Real solveForT(const DenseMatrix<Real> & reference_log10K, Real rhs) const;
};
