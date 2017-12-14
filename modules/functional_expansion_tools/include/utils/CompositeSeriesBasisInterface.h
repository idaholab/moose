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

#ifndef COMPOSITESERIESBASISINTERFACE_H
#define COMPOSITESERIESBASISINTERFACE_H

#include <iomanip>

#include "FunctionalBasisInterface.h"
#include "SingleSeriesBasisInterface.h"

class CompositeSeriesBasisInterface : public FunctionalBasisInterface
{
public:
  // default constructor
  CompositeSeriesBasisInterface();

  // non-default constructor
  CompositeSeriesBasisInterface(const std::vector<MooseEnum> & domains,
                                const std::vector<std::size_t> & orders,
                                std::vector<MooseEnum> series_types);

  // default destructor
  virtual ~CompositeSeriesBasisInterface() = default;

  // evaluate the orthonormal version of the composite series
  virtual void evaluateOrthonormal() final;

  // evaluate the standard version of the composite series
  virtual void evaluateStandard() final;

  // get the standardized functional basis limits by looping over each of the
  // single series
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const final;

  virtual Real getStandardizedFunctionVolume() const final;

  std::vector<Real> combineStandardizedFunctionLimits() const;

  // returns whether the cache is invalid
  virtual bool isCacheInvalid() const final;

  // determine if a point is in the bounds of the composite series
  virtual bool isInPhysicalBounds(const Point & point) const final;

  // set the _location and _standardized_location of each of the single series
  virtual void setLocation(const Point & p) final;

  // set the number of terms in the composite series
  void setNumberOfTerms();

  // this definition must be with CSBI because it has to loop over each of the
  // single series.
  virtual void setOrder(const std::vector<std::size_t> & orders) final;

  virtual void formatCoefficients(std::ostream & stream,
                                  const std::vector<Real> & coefficients) const;

protected:
  /// evaluates the values of _basis_evaluation for either evaluateOrthonormal
  /// or evaluateStandard
  void evaluateSeries(const std::vector<std::vector<Real>> & single_series_basis_evaluation);

  // series types in this composite series
  std::vector<MooseEnum> _series_types;

  // pointer to the single series type (one for each entry in _domains)
  std::vector<SingleSeriesBasisInterface *> _series;

private:
  // the previous point at which the series was evaluated
  Point _previous_point;

  // Hide _is_cache_invalid from subclasses - this is a candidate fro removal if CSBI is separated
  // from FBI
  using FBI::_is_cache_invalid;
};

#endif // COMPOSITESERIESBASISINTERFACE_H
