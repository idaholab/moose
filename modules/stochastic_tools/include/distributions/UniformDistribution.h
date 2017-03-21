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

#ifndef UNIFORMDISTRIBUTION_H
#define UNIFORMDISTRIBUTION_H

#include "Distribution.h"

class UniformDistribution;

template <>
InputParameters validParams<UniformDistribution>();
/**
 * A class used to generate uniform distribution
 */
class UniformDistribution : public Distribution
{
public:
  UniformDistribution(const InputParameters & parameters);
  virtual ~UniformDistribution();

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real inverseCdf(const Real & y) override;
  /// The lower bound for the uniform distribution
  const Real & _lower_bound;
  /// The upper bound for the uniform distribution
  const Real & _upper_bound;
};

#endif /* UNIFORMDISTRIBUTION_H */
