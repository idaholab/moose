//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseTypes.h"

// Shortened typename

/**
 * This class provides the basis for any custom functional basis, and is the parent class of both
 * SingleSeriesBasisInterface and CompositeSeriesBasisInterface
 */
class FunctionalBasisInterface
{
public:
  FunctionalBasisInterface();
  FunctionalBasisInterface(const unsigned int number_of_terms);

  /**
   * Returns the current evaluation at the given index
   */
  Real operator[](std::size_t index) const;

  /**
   * Returns an array reference containing the value of each generation term
   */
  const std::vector<Real> & getAllGeneration();

  /**
   * Returns an array reference containing the value of each expansion term
   */
  const std::vector<Real> & getAllExpansion();

  /**
   * Returns the number of terms in the series
   */
  std::size_t getNumberOfTerms() const;

  /**
   * Gets the last term of the generation functional basis
   */
  Real getGeneration();

  /**
   * Gets the sum of all terms in the generation functional basis
   */
  Real getGenerationSeriesSum();

  /**
   * Gets the #_order-th term of the expansion functional basis
   */
  Real getExpansion();

  /**
   * Evaluates the sum of all terms in the expansion functional basis up to #_order
   */
  Real getExpansionSeriesSum();

  /**
   * Returns a vector of the lower and upper bounds of the standard functional space
   */
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const = 0;

  /**
   * Returns the volume within the standardized function local_limits
   */
  virtual Real getStandardizedFunctionVolume() const = 0;

  /**
   * Returns true if the current evaluation is generation
   */
  bool isGeneration() const;

  /**
   * Returns true if the current evaluation is expansion
   */
  bool isExpansion() const;

  /**
   * Whether the cached values correspond to the current point
   */
  virtual bool isCacheInvalid() const = 0;

  /**
   * Determines if the point provided is in within the physical bounds
   */
  virtual bool isInPhysicalBounds(const Point & point) const = 0;

  /**
   * Set the location that will be used by the series to compute values
   */
  virtual void setLocation(const Point & point) = 0;

  /**
   * Set the order of the series
   */
  virtual void setOrder(const std::vector<std::size_t> & orders) = 0;

  /**
   * Sets the bounds of the series
   */
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) = 0;

  /// An enumeration of the domains available to each functional series
  static MooseEnum _domain_options;

protected:
  /**
   * Set all entries of the basis evaluation to zero.
   */
  virtual void clearBasisEvaluation(const unsigned int & number_of_terms);

  /**
   * Evaluate the generation form of the functional basis
   */
  virtual void evaluateGeneration() = 0;

  /**
   * Evaluate the expansion form of the functional basis
   */
  virtual void evaluateExpansion() = 0;

  /**
   * Helper function to load a value from #_series
   */
  Real load(std::size_t index) const;

  /**
   * Helper function to store a value in #_series
   */
  void save(std::size_t index, Real value);

  /// The number of terms in the series
  unsigned int _number_of_terms;

  /// indicates if the evaluated values correspond to the current location
  bool _is_cache_invalid;

private:
  /// Stores the values of the basis evaluation
  std::vector<Real> _basis_evaluation;

  /// Indicates whether the current evaluation is expansion or generation
  bool _is_generation;
};
