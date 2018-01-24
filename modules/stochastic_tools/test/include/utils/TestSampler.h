//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TESTSAMPLERMATERIAL_H
#define TESTSAMPLERMATERIAL_H

#include "ElementUserObject.h"
#include "SamplerInterface.h"
#include "Sampler.h"

// Forward Declarations
class TestSampler;

template <>
InputParameters validParams<TestSampler>();

/**
 * UserObject for testing Sampler object threaded and parallel behavior, it should be used for
 * anything else.
 */
class TestSampler : public ElementUserObject, public SamplerInterface
{
public:
  TestSampler(const InputParameters & parameters);

protected:
  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final;
  virtual void threadJoin(const UserObject & uo) final;
  Sampler & _sampler;
  const MooseEnum & _test_type;
};

#endif // TESTSAMPLERMATERIAL_H
