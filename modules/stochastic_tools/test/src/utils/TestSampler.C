//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSampler.h"

template <>
InputParameters
validParams<TestSampler>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<SamplerName>("sampler", "The sampler to test.");

  MooseEnum test_type("mpi thread");
  params.addParam<MooseEnum>("test_type", test_type, "The type of test to perform.");
  return params;
}

TestSampler::TestSampler(const InputParameters & parameters)
  : ElementUserObject(parameters),
    SamplerInterface(this),
    _sampler(getSampler("sampler")),
    _test_type(getParam<MooseEnum>("test_type"))
{
}

void
TestSampler::finalize()
{
  if (_communicator.size() > 1 && _test_type == "mpi")
  {
    std::size_t vec_size;
    std::vector<Real> samples;
    if (_communicator.rank() == 0)
    {
      samples = _sampler.getSamples()[0].get_values();
      vec_size = samples.size();
    }

    _communicator.broadcast(vec_size);
    samples.resize(vec_size);
    _communicator.broadcast(samples);

    if (_sampler.getSamples()[0].get_values() != samples)
      mooseError("The sample generation is not working correctly with MPI.");
  }
}

void
TestSampler::threadJoin(const UserObject & uo)
{
  if (_test_type == "thread")
  {
    const TestSampler & other = static_cast<const TestSampler &>(uo);
    if (_sampler.getSamples() != other._sampler.getSamples())
      mooseError("The sample generation is not working correctly with threads.");
  }
}
