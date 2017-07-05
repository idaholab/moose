/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TESTDISTRIBUTIONPOSTPROCESSOR_H
#define TESTDISTRIBUTIONPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class TestDistributionPostprocessor;
class Distribution;

template <>
InputParameters validParams<TestDistributionPostprocessor>();

/**
 * Test object for testing distribution capabilities.
 *
 * WARNING! This object is only for testing and should not be used in general.
 */
class TestDistributionPostprocessor : public GeneralPostprocessor
{
public:
  TestDistributionPostprocessor(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual PostprocessorValue getValue() override;

protected:
  /// Object of statistics distribution
  Distribution & _distribution;

  /// The value to supply to method
  const Real & _value;

  /// The distribution method to call
  const MooseEnum & _distribution_method;
};

#endif /* TESTDISTRIBUTIONPOSTPROCESSOR_H */
