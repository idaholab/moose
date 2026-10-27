//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVSetupCounter.h"
#include "InputParameters.h"

/**
 * Mixin that counts every SetupInterface method it receives, so that a test can assert through
 * FVSetupCount how many times the system actually dispatched each of them.
 *
 * The setup methods of a finite volume object that is not dispatched are inherited no-ops, so the
 * failure is silent; counting here is what makes it observable. Counting rather than merely
 * recording that a call happened is what lets a test catch a method being dispatched twice, which
 * is what happens when a query forgets to restrict itself to one solver system.
 *
 * \p Base is the finite volume family being tested, one per family so that every family is
 * covered.
 */
template <typename Base>
class FVSetupTester : public Base, public FVSetupCounter
{
public:
  static InputParameters validParams() { return Base::validParams(); }

  FVSetupTester(const InputParameters & params) : Base(params) {}

  virtual void initialSetup() override { incrementSetupCount("INITIAL"); }
  virtual void timestepSetup() override { incrementSetupCount("TIMESTEP"); }
  virtual void customSetup(const ExecFlagType & exec_type) override
  {
    incrementSetupCount("CUSTOM");
    // Many execution flags route through customSetup, most of them nothing to do with finite
    // volume, so a per-flag count is what a test can state an expected value for
    incrementSetupCount("CUSTOM_" + std::string(exec_type));
  }
};
