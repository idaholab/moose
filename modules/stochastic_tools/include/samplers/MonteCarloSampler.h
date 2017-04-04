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

#ifndef MONTECARLOSAMPLER_H
#define MONTECARLOSAMPLER_H

#include "Sampler.h"

class MonteCarloSampler;

template <>
InputParameters validParams<MonteCarloSampler>();
/**
 * A class used to perform Monte Carlo Sampling
 */
class MonteCarloSampler : public Sampler
{
public:
  MonteCarloSampler(const InputParameters & parameters);
  virtual ~MonteCarloSampler();

protected:
  virtual void generateSamples() override;
  unsigned int _num_samples;
};

#endif /* MONTECARLOSAMPLER_H */
