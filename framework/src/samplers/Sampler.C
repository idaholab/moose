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
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "The names of distributions that you want to sample.");
  params.addParam<std::vector<unsigned int>>("seeds",
                                             std::vector<unsigned int>(1, 19800624),
                                             "Random number generator initial seed(s), "
                                             "each seed will initialize a new generator "
                                             "in the MooseRandom object.");
  params.registerBase("Sampler");
  return params;
}

Sampler::Sampler(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    DistributionInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _seeds(getParam<std::vector<unsigned int>>("seeds"))
{
  for (std::size_t i = 0; i < _seeds.size(); ++i)
    _generator.seed(i, _seeds[i]);

  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));
}

void
Sampler::execute()
{
  // Get the samples then save the state so that subsequent calls to getSamples returns the same
  // random numbers until this execute command is called again.
  getSamples();
  _generator.saveState();
}

std::vector<std::vector<Real>>
Sampler::getSamples()
{
  _generator.restoreState();
  std::vector<std::vector<Real>> output(_distributions.size());
  for (std::size_t i = 0; i < _distributions.size(); ++i)
    output[i] = sampleDistribution(*_distributions[i]);
  return output;
}

void
Sampler::checkSeedNumber(unsigned int required) const
{
  if (_seeds.size() < required)
    mooseError("The '",
               name(),
               "' sampler requires ",
               required,
               " seeds but only ",
               _seeds.size(),
               " was provided.");
}
