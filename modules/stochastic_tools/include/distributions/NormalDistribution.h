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

#ifndef NORMALDISTRIBUTION_H
#define NORMALDISTRIBUTION_H

#include "Distribution.h"
#include "distribution_1D.h"

class NormalDistribution;

template <>
InputParameters validParams<NormalDistribution>();
/**
 * A class used to generate normal distribution from contrib RAVEN crow
 */
class NormalDistribution : public Distribution, BasicNormalDistribution
{
public:
  NormalDistribution(const InputParameters & parameters);
  virtual ~NormalDistribution();

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real inverseCdf(const Real & y) override;
};

#endif /* NORMALDISTRIBUTION_H */
