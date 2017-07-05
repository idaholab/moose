/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
