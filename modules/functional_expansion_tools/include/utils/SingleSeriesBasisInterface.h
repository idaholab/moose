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

#ifndef SINGLESERIESBASISINTERFACE_H
#define SINGLESERIESBASISINTERFACE_H

#include "FunctionalBasisInterface.h"

/**
 * This class is a simple wrapper around FunctionalBasisInterface, and intended
 * for use by any single functional series like Legendre, Zernike, etc...
 */
class SingleSeriesBasisInterface : public FunctionalBasisInterface
{
public:
  // default constructor
  SingleSeriesBasisInterface();

  // non-default constructor
  SingleSeriesBasisInterface(const std::vector<MooseEnum> & domains,
                             const std::vector<std::size_t> & orders,
                             const unsigned int number_of_terms);

  // default destructor
  virtual ~SingleSeriesBasisInterface() = default;

  // returns whether the cache is invalid for this series
  virtual bool isCacheInvalid() const final;

  // set the location at which to evaluate the series
  virtual void setLocation(const Point & point) final;

  // sets the order of the series
  virtual void setOrder(const std::vector<std::size_t> & orders) final;

  // sets the physical bounds of a single series
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) final;

  // get the standardized function limits for the single series
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const = 0;

  // pure virtual methods to be defined by derived classes ---------------------

  /**
   * Standardize the location according to the requirements of the underlying
   * basis, which may actually convert the Cartesian coordinates into a more
   * suitable system. The second version exists simply to return the value.
   */
  virtual std::vector<Real> getStandardizedLocation(const std::vector<Real> & location) const = 0;

  // returns the number of terms in the single series given a value for the order
  virtual std::size_t
  calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const = 0;

  // returns the order of the particular domain index
  std::size_t getOrder(std::size_t domain) const;

  // an ordered list of the x, y, and/or z domains needed by the functional basis
  // to convert a point to a standardized location
  const std::vector<MooseEnum> _domains;

protected:
  // the order of the series
  std::vector<std::size_t> _orders;

  // the physical bounds of the series
  std::vector<Real> _physical_bounds;

  // the standardized location of evaluation
  std::vector<Real> _standardized_location;

  // Checks the physical bounds according to the actual implementation
  virtual void checkPhysicalBounds(const std::vector<Real> & bounds) const = 0;

  // convert a spatial point to a location that the series will use to determine
  // the value at which to evaluate the series
  std::vector<Real> extractLocationFromPoint(const Point & point) const;

private:
  /// flag for if the physical bounds are specified for this series
  bool _are_physical_bounds_specified;

  /**
   * The domain locations of the current evaluation.
   * This is private so that derived classes will be required to use
   * #_standardized_location, essentially forcing location-awareness compliance
   */
  std::vector<Real> _location;

  // Hide _is_cache_invalid from subclasses
  using FBI::_is_cache_invalid;
};

#endif // SINGLESERIESBASISINTERFACE_H
