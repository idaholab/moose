//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
class Transient;

/**
 * Report the time and iteration information for the simulation.
 */
class IterationInfo : public GeneralReporter
{
public:
  static InputParameters validParams();
  IterationInfo(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  const MultiMooseEnum & _items;

  // Reporter values to return (all are computed as "replicated" values)
  Real & _time_value;
  unsigned int & _time_step_value;
  unsigned int & _num_linear;
  unsigned int & _num_nonlinear;
  unsigned int & _num_fixed_point;

  // Used to allow for optional declare
  Real _dummy_real = 0;
  unsigned int _dummy_unsigned_int = 0;

  // Helper to perform optional declaration based on "_items"
  template <typename T>
  T & declareHelper(const std::string & item_name, T & _dummy, bool extra_check = true);
};

template <typename T>
T &
IterationInfo::declareHelper(const std::string & item_name, T & dummy, bool extra_check)
{
  return (extra_check && (!_items.isValid() || _items.contains(item_name)))
             ? declareValueByName<T>(item_name, REPORTER_MODE_REPLICATED)
             : dummy;
}
