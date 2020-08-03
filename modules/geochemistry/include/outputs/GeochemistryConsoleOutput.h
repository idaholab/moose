//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemistryReactorBase.h"
#include "Output.h"
#include "UserObjectInterface.h"
#include "NearestNodeNumberUO.h"

/**
 * Outputs information (to the console) from a GeochemistryReactorBase at a point
 */
class GeochemistryConsoleOutput : public Output, public UserObjectInterface
{
public:
  /// contains params that are shared with Actions that use this object
  static InputParameters sharedParams();

  static InputParameters validParams();

  GeochemistryConsoleOutput(const InputParameters & parameters);

protected:
  virtual void output(const ExecFlagType & type) override;

  /// the Reactor from which to extract info
  const GeochemistryReactorBase & _reactor;
  /// UserObject defining the node of interest
  const NearestNodeNumberUO & _nnn;
  /// precision of output
  const unsigned _precision;
  /// Tolerance on stoichiometric coefficients before they are deemed to be zero
  const Real _stoi_tol;
  /// Whether to print solver info
  const bool _solver_info;
  /// Species with molalities less than mol_cutoff will not be outputted
  const Real _mol_cutoff;

private:
  void outputNernstInfo(const GeochemicalSystem & egs_ref) const;
};
