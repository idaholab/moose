//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <iomanip>

#include "FunctionalBasisInterface.h"

#include "libmesh/point.h"

class SingleSeriesBasisInterface;

/**
 * This class is the basis for constructing a composite---or convolved---functional series by
 * combining multiple other series together. Nonseparability is currently assumed.
 */
class CompositeSeriesBasisInterface : public FunctionalBasisInterface
{
public:
  CompositeSeriesBasisInterface(const std::string & who_is_using_me);
  CompositeSeriesBasisInterface(const std::vector<std::size_t> & orders,
                                std::vector<MooseEnum> series_types,
                                const std::string & who_is_using_me);
  virtual ~CompositeSeriesBasisInterface();

  // Disable move and copy operations
  CompositeSeriesBasisInterface(const CompositeSeriesBasisInterface &) = delete;
  CompositeSeriesBasisInterface(CompositeSeriesBasisInterface &&) = delete;
  void operator=(const CompositeSeriesBasisInterface &) = delete;
  CompositeSeriesBasisInterface & operator=(CompositeSeriesBasisInterface &&) = delete;

  // Overrides from FunctionalBasisInterface
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const final;
  virtual Real getStandardizedFunctionVolume() const final;
  virtual bool isCacheInvalid() const final;
  virtual bool isInPhysicalBounds(const Point & point) const final;
  virtual void setLocation(const Point & p) final;
  // This definition must be with CSBI because it has to loop over each of the single series.
  virtual void setOrder(const std::vector<std::size_t> & orders) final;

  /**
   * Get the function limits by looping over each of the single series
   */
  std::vector<Real> combineStandardizedFunctionLimits() const;

  /**
   * Initialize the number of terms in the composite series by looping over the single series
   */
  void setNumberOfTerms();

  /**
   * Appends a tabulated form of the coefficients to the stream
   */
  virtual void formatCoefficients(std::ostream & stream,
                                  const std::vector<Real> & coefficients) const;

protected:
  // Overrides from FunctionalBasisInterface
  virtual void evaluateGeneration() final;
  virtual void evaluateExpansion() final;

  /**
   * Evaluates the values of _basis_evaluation for either evaluateGeneration() or
   * evaluateExpansion()
   */
  void evaluateSeries(const std::vector<std::vector<Real>> & single_series_basis_evaluations);

  /// The series types in this composite series
  std::vector<MooseEnum> _series_types;

  /// A pointer to the single series type (one for each entry in _domains)
  std::vector<std::unique_ptr<SingleSeriesBasisInterface>> _series;

  /// The name of the MooseObject that is using this class
  const std::string & _who_is_using_me;

private:
  /// The previous point at which the series was evaluated
  Point _previous_point;

  // Hide from subclasses (everything can be done by CSBI) to prevent BAD things from happening
  using FunctionalBasisInterface::_is_cache_invalid;
  using FunctionalBasisInterface::clearBasisEvaluation;
};
