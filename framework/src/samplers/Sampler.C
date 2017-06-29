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
  _generator.seed(0, _seed);
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));
  setNumberOfRequiedRandomSeeds(1);
}

void
Sampler::execute()
{
  // Get the samples then save the state so that subsequent calls to getSamples returns the same
  // random numbers until this execute command is called again.
  getSamples();
  _generator.saveState();
}

std::vector<DenseMatrix<Real>>
Sampler::getSamples()
{
  _generator.restoreState();
  sampleSetUp();
  std::vector<DenseMatrix<Real>> output(_distributions.size());
  for (auto i = beginIndex(_distributions); i < _distributions.size(); ++i)
    output[i] = sampleDistribution(*_distributions[i], i);
  sampleTearDown();
  return output;
}

double
Sampler::rand(const unsigned int index)
{
  mooseAssert(index < _seeds.size(), "The seed number index does not exists.");
  return _generator.rand(_seeds[index]);
}

void
Sampler::setNumberOfRequiedRandomSeeds(const std::size_t & number)
{
  if (number == 0)
    mooseError("The number of seeds must be larger than zero.");
  _seeds.resize(number);
  _seeds[0] = _seed;
  _generator.seed(0, _seed);
  for (std::size_t i = 1; i < number; ++i)
  {
    _seeds[i] = _seeds[i - 1] + 1;
    _generator.seed(i, _seeds[i]);
  }
}
