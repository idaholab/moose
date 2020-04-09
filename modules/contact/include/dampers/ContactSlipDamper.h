//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "GeneralDamper.h"

// Forward Declarations
class AuxiliarySystem;
class DisplacedProblem;
class PenetrationLocator;

/**
 * Simple constant damper.
 *
 * Modifies the non-linear step by applying a constant damping factor
 */
class ContactSlipDamper : public GeneralDamper
{
public:
  static InputParameters validParams();

  ContactSlipDamper(const InputParameters & parameters);

  virtual void timestepSetup();

protected:
  AuxiliarySystem & _aux_sys;
  MooseSharedPointer<DisplacedProblem> _displaced_problem;

  /**
   * Compute the amount of damping
   */
  virtual Real computeDamping(const NumericVector<Number> & solution,
                              const NumericVector<Number> & update);

  /**
   * Determine whether the damper should operate on the interaction corresponding to the supplied
   * PenetrationLocator.
   */
  bool operateOnThisInteraction(const PenetrationLocator & pen_loc);

  std::set<std::pair<int, int>> _interactions;

  int _num_contact_nodes;
  int _num_sticking;
  int _num_slipping;
  int _num_slipping_friction;
  int _num_stick_locked;
  int _num_slip_reversed;
  Real _max_iterative_slip;
  Real _min_damping_factor;
  Real _damping_threshold_factor;
  bool _debug_output;

  /// Convenient typedef for frequently used iterator
  typedef std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator
      pl_iterator;
};
