//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "GeneralReporter.h"
#include "Times.h"

/**
 * Times objects are under the hood Reporters, but limited to a vector of Real
 */
class TimesReporter : public GeneralReporter, public Times
{
public:
  static InputParameters validParams();
  TimesReporter(const InputParameters & parameters);
  virtual ~TimesReporter() = default;

protected:
  /// In charge of computing / loading the times, unless all that could be done there is done
  /// in the constructor
  virtual void initialize() override = 0;

  /// By default, we wont execute often but "executing" will mean loading the times
  virtual void execute() override { initialize(); }

  /// In charge of reduction across all ranks
  virtual void finalize() override;

  /// By default, Times will not be modified very regularly
  virtual void timestepSetup() override {}
  virtual void residualSetup() override {}
  virtual void jacobianSetup() override {}

  /// Whether generation of times is distributed or not (and therefore needs a broadcast)
  const bool _need_broadcast;
  /// Whether times should be sorted, because they come from different sources for example
  const bool _need_sort;
  /// Whether duplicate times should be removed
  const bool _need_unique;
  /// Absolute tolerance for performing duplication checks to make the times vector unique
  const Real _unique_tol;
};
