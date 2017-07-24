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
    _seed(getParam<unsigned int>("seed"))
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
Sampler::setSampleNames(std::vector<std::string> names)
{
  _sample_names = names;
  mooseAssert(getSamples().size() == _sample_names.size(),
              "The number of sample names must match the number of samples returned.");
}

Sampler::Location
Sampler::getLocation(unsigned int global_index)
{
  if (_offsets.empty())
    reinit(getSamples());

  std::vector<unsigned int>::const_iterator iter;
  iter = std::upper_bound(_offsets.begin(), _offsets.end(), global_index) - 1;
  return Sampler::Location(iter - _offsets.begin(), global_index - *iter);
}

unsigned int
Sampler::getTotalNumberOfRows()
{
  if (_total_rows == 0)
    reinit(getSamples());
  return _total_rows;
}
