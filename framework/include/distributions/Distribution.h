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

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include "MooseObject.h"

class Distribution;

template <>
InputParameters validParams<Distribution>();

/**
 * All Distributions should inherit from this class
 */
class Distribution : public MooseObject
{
public:
  Distribution(const InputParameters & parameters);
  /**
   * Compute the probability with given probability distribution function (PDF) at x
   */
  virtual Real pdf(const Real & x) = 0;

  /**
   * Compute the cumulative probability with given cumulative probability distribution (CDF) at x
   */
  virtual Real cdf(const Real & x) = 0;

  /**
   * Compute the inverse CDF (quantile function) value for given variable value y
   */
  virtual Real quantile(const Real & y) = 0;
};

#endif /* DISTRIBUTION_H */
