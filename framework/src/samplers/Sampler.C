//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// STL includes
#include <iterator>

// MOOSE includes
#include "Sampler.h"
#include "MooseRandom.h"
#include "Distribution.h"

template <>
InputParameters
validParams<Sampler>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<SetupInterface>();
  params += validParams<DistributionInterface>();

  params.addClassDescription("A base class for distribution sampling.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "The names of distributions that you want to sample.");
  params.addParam<unsigned int>("seed", 0, "Random number generator initial seed");
  params.registerBase("Sampler");
  return params;
}

Sampler::Sampler(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    DistributionInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _seed(getParam<unsigned int>("seed")),
    _total_rows(0)
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));
  setNumberOfRequiedRandomSeeds(1);
}

void
Sampler::execute()
{
  // Get the samples then save the state so that subsequent calls to getSamples returns the same
  // random numbers until this execute command is called again.
  std::vector<DenseMatrix<Real>> data = getSamples();
  _generator.saveState();
  reinit(data);
}

void
Sampler::reinit(const std::vector<DenseMatrix<Real>> & data)
{
  // Update offsets and total number of rows
  _total_rows = 0;
  _offsets.clear();
  _offsets.reserve(data.size() + 1);
  _offsets.push_back(_total_rows);
  for (const DenseMatrix<Real> & mat : data)
  {
    _total_rows += mat.m();
    _offsets.push_back(_total_rows);
  }
}

std::vector<DenseMatrix<Real>>
Sampler::getSamples()
{
  _generator.restoreState();
  sampleSetUp();
  std::vector<DenseMatrix<Real>> output = sample();
  sampleTearDown();

  if (_sample_names.empty())
  {
    _sample_names.resize(output.size());
    for (auto i = beginIndex(output); i < output.size(); ++i)
      _sample_names[i] = "sample_" + std::to_string(i);
  }
  mooseAssert(output.size() == _sample_names.size(),
              "The number of sample names must match the number of samples returned.");

  mooseAssert(output.size() > 0,
              "It is not acceptable to return an empty vector of sample matrices.");

  return output;
}

double
Sampler::rand(const unsigned int index)
{
  mooseAssert(index < _generator.size(), "The seed number index does not exists.");
  return _generator.rand(index);
}

void
Sampler::setNumberOfRequiedRandomSeeds(const std::size_t & number)
{
  if (number == 0)
    mooseError("The number of seeds must be larger than zero.");

  // Seed the "master" seed generator
  _seed_generator.seed(0, _seed);

  // See the "slave" generator that will be used for the random number generation
  for (std::size_t i = 0; i < number; ++i)
    _generator.seed(i, _seed_generator.randl(0));

  _generator.saveState();
}

void
Sampler::setSampleNames(const std::vector<std::string> & names)
{
  _sample_names = names;

  // Use assert because to check the size a getSamples call is required, which you don't
  // want to do if you don't need it.
  mooseAssert(getSamples().size() == _sample_names.size(),
              "The number of sample names must match the number of samples returned.");
}

Sampler::Location
Sampler::getLocation(unsigned int global_index)
{
  if (_offsets.empty())
    reinit(getSamples());

  mooseAssert(_offsets.size() > 1,
              "The getSamples method returned an empty vector, if you are seeing this you have "
              "done something to bypass another assert in the 'getSamples' method that should "
              "prevent this message.");

  // The lower_bound method returns the first value "which does not compare less than" the value and
  // upper_bound performs "which compares greater than." The upper_bound -1 method is used here
  // because lower_bound will provide the wrong index, but the method here will provide the correct
  // index, set the Sampler.GetLocation test in moose/unit/src/Sampler.C for an example.
  std::vector<unsigned int>::iterator iter =
      std::upper_bound(_offsets.begin(), _offsets.end(), global_index) - 1;
  return Sampler::Location(std::distance(_offsets.begin(), iter), global_index - *iter);
}

unsigned int
Sampler::getTotalNumberOfRows()
{
  if (_total_rows == 0)
    reinit(getSamples());
  return _total_rows;
}
