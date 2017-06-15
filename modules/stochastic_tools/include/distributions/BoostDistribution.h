/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef BOOSTDISTRIBUTION_H
#define BOOSTDISTRIBUTION_H

#include "Distribution.h"

#ifdef LIBMESH_HAVE_BOOST

#include <boost/math/distributions.hpp>

/**
 * A class used to as a base for distributions defined by Boost.
 */
template <typename T>
class BoostDistribution : public Distribution
{
public:
  BoostDistribution(const InputParameters & parameters);

protected:
  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real quantile(const Real & y) override;

  /// This must be defined by the child class in the constructor
  std::unique_ptr<T> _boost_distribution;
};

template <typename T>
BoostDistribution<T>::BoostDistribution(const InputParameters & parameters)
  : Distribution(parameters)
{
}

template <typename T>
Real
BoostDistribution<T>::pdf(const Real & x)
{
  mooseAssert(_boost_distribution, "Boost distribution pointer not defined.");
  return boost::math::pdf(*_boost_distribution, x);
}

template <typename T>
Real
BoostDistribution<T>::cdf(const Real & x)
{
  mooseAssert(_boost_distribution, "Boost distribution pointer not defined.");
  return boost::math::cdf(*_boost_distribution, x);
}

template <typename T>
Real
BoostDistribution<T>::quantile(const Real & y)
{
  mooseAssert(_boost_distribution, "Boost distribution pointer not defined.");
  return boost::math::quantile(*_boost_distribution, y);
}

#endif // LIBMESH_HAVE_BOOST
#endif // BOOSTDISTRIBUTION_H
