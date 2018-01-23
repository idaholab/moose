// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPOSITESERIESBASISINTERFACE_H
#define COMPOSITESERIESBASISINTERFACE_H

#include <iomanip>

#include "FunctionalBasisInterface.h"
#include "SingleSeriesBasisInterface.h"

/**
 * This class is the basis for consructing a composite---or convolved---functional series by
 * combining multiple other series together. Nonseparability is currently assumed.
 */
class CompositeSeriesBasisInterface : public FunctionalBasisInterface
{
public:
  CompositeSeriesBasisInterface();
  CompositeSeriesBasisInterface(const std::vector<std::size_t> & orders,
                                std::vector<MooseEnum> series_types);

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
  virtual void evaluateOrthonormal() final;
  virtual void evaluateStandard() final;

  /**
   * Evaluates the values of _basis_evaluation for either evaluateOrthonormal() or
   * evaluateStandard()
   */
  void evaluateSeries(const std::vector<std::vector<Real>> & single_series_basis_evaluations);

  /// The series types in this composite series
  std::vector<MooseEnum> _series_types;

  /// A pointer to the single series type (one for each entry in _domains)
  std::vector<SingleSeriesBasisInterface *> _series;

private:
  /// The previous point at which the series was evaluated
  Point _previous_point;

  // Hide _is_cache_invalid from subclasses, as everything can be done by CSBI
  using FBI::_is_cache_invalid;
};

#endif // COMPOSITESERIESBASISINTERFACE_H
