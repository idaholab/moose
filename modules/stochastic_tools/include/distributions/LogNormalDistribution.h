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

#ifndef LOGNORMALDISTRIBUTION_H
#define LOGNORMALDISTRIBUTION_H

#include "Distribution.h"
#include "distribution_1D.h"

class LogNormalDistribution;

template <>
InputParameters validParams<LogNormalDistribution>();
/**
 * A class used to generate lognormal distribution from RAVEN crow
 */
class LogNormalDistribution : public Distribution, BasicLogNormalDistribution
{
public:
  LogNormalDistribution(const InputParameters & parameters);
  virtual ~LogNormalDistribution();

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real inverseCdf(const Real & y) override;
};

#endif /* LOGNORMALDISTRIBUTION_H */
