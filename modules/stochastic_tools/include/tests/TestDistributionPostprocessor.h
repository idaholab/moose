/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef TESTDISTRIBUTIONPOSTPROCESSOR_H
#define TESTDISTRIBUTIONPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class TestDistributionPostprocessor;
class Distribution;

template <>
InputParameters validParams<TestDistributionPostprocessor>();

/**
 * This postprocessor is used to test the distribution capabilities.
 */
class TestDistributionPostprocessor : public GeneralPostprocessor
{
public:
  TestDistributionPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// Object of statistics distribution
  Distribution & _distribution;
  /// The value of cdf for given distribution
  const Real & _cdf_value;
  /// The value for the random variable of given distribution
  const Real & _sample_value;
};

#endif /* TESTDISTRIBUTIONPOSTPROCESSOR_H */
