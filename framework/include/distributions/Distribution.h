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
#include "MooseRandom.h"
#include "Restartable.h"

class Distribution;

template <>
InputParameters validParams<Distribution>();

/**
 * All Distributions should inherit from this class
 */
class Distribution : public MooseObject, public Restartable
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
   * Compute the inverse CDF value for given variable value y
   */
  virtual Real inverseCdf(const Real & y) = 0;
  /**
   * Get the random number from given distribution
   */
  virtual Real getRandomNumber();
  /**
   * Get the seed of random number generator
   */
  unsigned int getSeed();
  /**
   * Set the seed for the random number generator
   */
  virtual void setSeed(unsigned int seed);

protected:
  THREAD_ID _tid;

  /// the seed for the random number generator
  unsigned int _seed;
  /// Object of random number generator
  MooseRandom & _random;
};

#endif /* DISTRIBUTION_H */
