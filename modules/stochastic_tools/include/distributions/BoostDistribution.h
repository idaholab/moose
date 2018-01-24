//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BOOSTDISTRIBUTION_H
#define BOOSTDISTRIBUTION_H

#include "Distribution.h"

#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
#include <boost/math/distributions.hpp>
#else
class BoostDistributionDummy
{
public:
  BoostDistributionDummy(Real...) {}
};
namespace boost
{
namespace math
{
template <typename T>
using weibull_distribution = BoostDistributionDummy;

template <typename T>
using normal_distribution = BoostDistributionDummy;

template <typename T>
using lognormal_distribution = BoostDistributionDummy;
}
}
#endif

/**
 * A class used to as a base for distributions defined by Boost.
 *
 * The default type is set to Real to allow for derived classes to compile without Boost and
 * trigger the mooseError in the constructor.
 */
template <typename T = Real>
class BoostDistribution : public Distribution
{
public:
  BoostDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real quantile(const Real & y) override;

protected:
  /// This must be defined by the child class in the constructor
  std::unique_ptr<T> _distribution_unique_ptr;
};

template <typename T>
BoostDistribution<T>::BoostDistribution(const InputParameters & parameters)
  : Distribution(parameters)
{
#ifndef LIBMESH_HAVE_EXTERNAL_BOOST
  mooseError("The ",
             getParam<std::string>("type"),
             " distribution named '",
             name(),
             "' requires that libMesh be compiled with an external Boost library, this may be done "
             "using the --with-boost configure option.");
#endif
}

template <typename T>
Real
BoostDistribution<T>::pdf(const Real & x)
{
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  mooseAssert(_distribution_unique_ptr, "Boost distribution pointer not defined.");
  return boost::math::pdf(*_distribution_unique_ptr, x);
#else
  return x; // unreachable
#endif
}

template <typename T>
Real
BoostDistribution<T>::cdf(const Real & x)
{
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  mooseAssert(_distribution_unique_ptr, "Boost distribution pointer not defined.");
  return boost::math::cdf(*_distribution_unique_ptr, x);
#else
  return x; // unreachable
#endif
}

template <typename T>
Real
BoostDistribution<T>::quantile(const Real & y)
{
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  mooseAssert(_distribution_unique_ptr, "Boost distribution pointer not defined.");
  return boost::math::quantile(*_distribution_unique_ptr, y);
#else
  return y; // unreachable
#endif
}

#endif // BOOSTDISTRIBUTION_H
