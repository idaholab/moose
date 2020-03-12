//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionalBasisInterface.h"
#include <functional>

/**
 * This class is a simple wrapper around FunctionalBasisInterface, and intended for use by any
 * single functional series like Legendre, Zernike, etc...
 */
class SingleSeriesBasisInterface : public FunctionalBasisInterface
{
public:
  SingleSeriesBasisInterface();
  SingleSeriesBasisInterface(const std::vector<MooseEnum> & domains,
                             const std::vector<std::size_t> & orders,
                             const unsigned int number_of_terms);
  virtual ~SingleSeriesBasisInterface();

  // Disable move and copy operations
  SingleSeriesBasisInterface(const SingleSeriesBasisInterface &) = delete;
  SingleSeriesBasisInterface(SingleSeriesBasisInterface &&) = delete;
  void operator=(const SingleSeriesBasisInterface &) = delete;
  SingleSeriesBasisInterface & operator=(SingleSeriesBasisInterface &&) = delete;

  // Overrides from FunctionalBasisInterface
  virtual bool isCacheInvalid() const final;
  virtual void setLocation(const Point & point) final;
  virtual void setOrder(const std::vector<std::size_t> & orders) final;
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) final;

  /**
   * Returns the number of terms in the single series given a value for the order
   */
  virtual std::size_t
  calculatedNumberOfTermsBasedOnOrder(const std::vector<std::size_t> & order) const = 0;

  /**
   * Standardize the location according to the requirements of the underlying basis, which may
   * actually convert the Cartesian coordinates into a more suitable system. The second version
   * exists simply to return the value.
   */
  virtual std::vector<Real> getStandardizedLocation(const std::vector<Real> & location) const = 0;

  /**
   * Returns the order of the particular domain index
   */
  std::size_t getOrder(std::size_t domain) const;

  /// An ordered list of the x, y, and/or z domains needed by the functional basis to convert a point
  /// to a standardized location
  const std::vector<MooseEnum> _domains;

protected:
  // Overrides from FunctionalBasisInterface
  virtual void evaluateGeneration() override;
  virtual void evaluateExpansion() override;

  /**
   * Checks the physical bounds according to the actual implementation
   */
  virtual void checkPhysicalBounds(const std::vector<Real> & bounds) const = 0;

  /**
   * Convert a spatial point to a location that the series will use to determine the value at which
   * to evaluate the series
   */
  std::vector<Real> extractLocationFromPoint(const Point & point) const;

  /// The order of the series
  std::vector<std::size_t> _orders;

  /// The physical bounds of the series
  std::vector<Real> _physical_bounds;

  /// The standardized location of evaluation
  std::vector<Real> _standardized_location;

  /// The expansion evaluation wrapper
  std::function<void()> _evaluateExpansionWrapper;

  /// The generation evaluation wrapper
  std::function<void()> _evaluateGenerationWrapper;

private:
  /// Flag for if the physical bounds are specified for this series
  bool _are_physical_bounds_specified;

  /// The domain locations of the current evaluation. This is private so that derived classes will be
  /// required to use #_standardized_location, essentially forcing location-awareness compliance
  std::vector<Real> _location;

  // Hide from subclasses to prevent BAD things from happening
  using FunctionalBasisInterface::_is_cache_invalid;
  using FunctionalBasisInterface::clearBasisEvaluation;
};
