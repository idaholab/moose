/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
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

  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real quantile(const Real & y) override;

protected:
  /// The lower bound for the uniform distribution
  const Real & _lower_bound;

  /// The upper bound for the uniform distribution
  const Real & _upper_bound;
};

#endif /* UNIFORMDISTRIBUTION_H */
