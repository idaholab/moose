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

#ifndef WEIBULLDISTRIBUTION_H
#define WEIBULLDISTRIBUTION_H

#include "Distribution.h"
#include "distribution_1D.h"

class UniformDistribution;

template <>
InputParameters validParams<UniformDistribution>();
/**
 * A class used to generate a weibull distribution from contrib RAVEN crow
 */
class WeibullDistribution : public Distribution, BasicWeibullDistribution
{
public:
  WeibullDistribution(const InputParameters & parameters);
  virtual ~WeibullDistribution();

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real inverseCdf(const Real & y) override;
};

#endif /* WEIBULLDISTRIBUTION_H */
