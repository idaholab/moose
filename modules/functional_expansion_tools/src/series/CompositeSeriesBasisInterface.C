//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositeSeriesBasisInterface.h"
#include "Legendre.h"

#include <memory>

/*
 * Default constructor creates a functional basis with one term. In order for the _series member to
 * be initialized, we initialized it with a Legendre series.
 */
CompositeSeriesBasisInterface::CompositeSeriesBasisInterface(const std::string & who_is_using_me)
  : FunctionalBasisInterface(1), _who_is_using_me(who_is_using_me)
{
  _series.push_back(std::make_unique<Legendre>());
}

/*
 * The non-default constructor is where we actually loop over the series_types and initialize
 * pointers to those members. Because we won't know the number of terms until the end of the body
 * of the constructor, we need to call the default FunctionalBasisInterface constructor.
 */
CompositeSeriesBasisInterface::CompositeSeriesBasisInterface(
    const std::vector<std::size_t> & orders,
    std::vector<MooseEnum> series_types,
    const std::string & who_is_using_me)
  : FunctionalBasisInterface(), _series_types(series_types), _who_is_using_me(who_is_using_me)
{
  if (orders.size() != _series_types.size())
    mooseError(_who_is_using_me,
               " calling CSBI::CSBI(...): Incorrect number of 'orders' specified for "
               "'FunctionSeries'! Check that 'orders' is the correct length and no invalid "
               "enumerations are specified for the series.");
}

void
CompositeSeriesBasisInterface::evaluateGeneration()
{
  /*
   * Evaluate the generation versions of each of the single series, and collect the results before
   * passing them to evaluateSeries, where they will be multiplied together correctly and stored in
   * the composite series basis evaluation.
   */
  std::vector<std::vector<Real>> single_series_basis_evaluation;
  for (auto & series : _series)
    single_series_basis_evaluation.push_back(series->getAllGeneration());

  evaluateSeries(single_series_basis_evaluation);
}

void
CompositeSeriesBasisInterface::evaluateSeries(
    const std::vector<std::vector<Real>> & single_series_basis_evaluations)
{
  /*
   * Appropriate number of loops based on 1-D, 2-D, or 3-D to multiply the basis evaluations of the
   * single series together to form the basis evaluation of the entire composite series.
   */
  Real f1, f2, f3;
  std::size_t term = 0;

  if (single_series_basis_evaluations.size() == 1)
    for (std::size_t i = 0; i < _series[0]->getNumberOfTerms(); ++i, ++term)
      save(term, single_series_basis_evaluations[0][i]);

  if (single_series_basis_evaluations.size() == 2)
    for (std::size_t i = 0; i < _series[0]->getNumberOfTerms(); ++i)
    {
      f1 = single_series_basis_evaluations[0][i];
      for (std::size_t j = 0; j < _series[1]->getNumberOfTerms(); ++j, ++term)
      {
        f2 = single_series_basis_evaluations[1][j];
        save(term, f1 * f2);
      }
    }

  if (single_series_basis_evaluations.size() == 3)
    for (std::size_t i = 0; i < _series[0]->getNumberOfTerms(); ++i)
    {
      f1 = single_series_basis_evaluations[0][i];
      for (std::size_t j = 0; j < _series[1]->getNumberOfTerms(); ++j)
      {
        f2 = single_series_basis_evaluations[1][j];
        for (std::size_t k = 0; k < _series[2]->getNumberOfTerms(); ++k, ++term)
        {
          f3 = single_series_basis_evaluations[2][k];
          save(term, f1 * f2 * f3);
        }
      }
    }
}

void
CompositeSeriesBasisInterface::evaluateExpansion()
{
  /*
   * Evaluate the expansion versions of each of the single series, and collect the results before
   * passing them to evaluateSeries, where they will be multiplied together correctly and stored in
   * the composite series basis evaluation.
   */
  std::vector<std::vector<Real>> single_series_basis_evaluation;
  for (auto & series : _series)
    single_series_basis_evaluation.push_back(series->getAllExpansion());

  evaluateSeries(single_series_basis_evaluation);
}

const std::vector<Real> &
CompositeSeriesBasisInterface::getStandardizedFunctionLimits() const
{
  static const std::vector<Real> function_limits = combineStandardizedFunctionLimits();

  return function_limits;
}

Real
CompositeSeriesBasisInterface::getStandardizedFunctionVolume() const
{
  Real function_volume = 1.0;

  for (auto & series : _series)
    function_volume *= series->getStandardizedFunctionVolume();

  return function_volume;
}

std::vector<Real>
CompositeSeriesBasisInterface::combineStandardizedFunctionLimits() const
{
  std::vector<Real> function_limits;

  for (auto & series : _series)
  {
    std::vector<Real> local_limits = series->getStandardizedFunctionLimits();
    for (auto & limit : local_limits)
      function_limits.push_back(limit);
  }

  return function_limits;
}

void
CompositeSeriesBasisInterface::formatCoefficients(std::ostream & stream,
                                                  const std::vector<Real> & coefficients) const
{
  // clang-format off
  std::ostringstream formatted, domains, orders;
  std::size_t term = 0;

  stream <<               "---------------- Coefficients ----------------\n"
         <<               "               == Subindices ==\n";

  if (_series_types.size() == 1)
  {
    orders <<             "         Orders: " << std::setw(3) << _series[0]->getOrder(0) << "\n";
    domains <<            " == Index  ==    " << std::setw(3) << _series[0]->_domains[0]
            <<                                "             === Value ===\n"
            <<            "----------------------------------------------\n";

    for (std::size_t i = 0; i < _series[0]->getNumberOfTerms(); ++i, ++term)
      formatted <<        "    " << std::setw(4) << term
                <<                "         " << std::setw(3) << i
                <<                            "             " << std::setw(12) << coefficients[term] << "\n";
  }
  else if (_series_types.size() == 2)
  {
    orders <<             "         Orders: " << std::setw(3) << _series[0]->getOrder(0)
           <<                                 " " << std::setw(3) << _series[1]->getOrder(0) << "\n";
    domains <<            " == Index  ==    " << std::setw(3) << _series[0]->_domains[0]
            <<                                " " << std::setw(3) << _series[1]->_domains[0]
            <<                                   "          === Value ===\n"
            <<            "----------------------------------------------\n";

    for (std::size_t i = 0; i < _series[0]->getNumberOfTerms(); ++i)
    {
      for (std::size_t j = 0; j < _series[1]->getNumberOfTerms(); ++j, ++term)
        formatted <<      "    " << std::setw(4) << term
                  <<              "         " << std::setw(3) << i
                  <<                          " " << std::setw(3) << j
                  <<                              "         " << std::setw(12) << coefficients[term] << "\n";
    }
  }
  else if (_series_types.size() == 3)
  {
    orders <<             "         Orders: " << std::setw(3) << _series[0]->getOrder(0)
           <<                                 " " << std::setw(3) << _series[1]->getOrder(0)
           <<                                     " " << std::setw(3) << _series[2]->getOrder(0) << "\n";
    domains <<            " == Index  ==    " << std::setw(3) << _series[0]->_domains[0]
            <<                                " " << std::setw(3) << _series[1]->_domains[0]
            <<                                    " " << std::setw(3) << _series[2]->_domains[0]
            <<                                        "     === Value ===\n"
            <<            "----------------------------------------------\n";

    for (std::size_t i = 0; i < _series[0]->getNumberOfTerms(); ++i)
    {
      for (std::size_t j = 0; j < _series[1]->getNumberOfTerms(); ++j)
      {
        for (std::size_t k = 0; k < _series[2]->getNumberOfTerms(); ++k, ++term)
          formatted <<    "    " << std::setw(4) << term
                    <<            "         " << std::setw(3) << i
                    <<                        " " << std::setw(3) << j
                    <<                            " " << std::setw(3) << k
                    <<                                "     " << std::setw(12) << coefficients[term] << "\n";
      }
    }
  }
  // clang-format on

  stream << orders.str() << domains.str() << formatted.str() << std::flush;
}

bool
CompositeSeriesBasisInterface::isCacheInvalid() const
{
  /*
   * If any one of the single series have an invalid cache, then we need to re-evaluate the entire
   * composite series because the terms are multiplied.
   */
  for (auto & series : _series)
    if (series->isCacheInvalid())
      return true;

  return false;
}

bool
CompositeSeriesBasisInterface::isInPhysicalBounds(const Point & point) const
{
  /*
   * A point is in the physical bounds of the composite series if it is in the physical bounds of
   * each of the single series
   */
  for (auto & series : _series)
    if (!series->isInPhysicalBounds(point))
      return false;

  return true;
}

void
CompositeSeriesBasisInterface::setNumberOfTerms()
{
  unsigned int number_of_terms = 1;

  // Accumulate the number of terms for each series
  for (auto & series : _series)
    number_of_terms *= series->getNumberOfTerms();

  _number_of_terms = number_of_terms;

  /*
   * The length of the _basis_evaluation depends on the number of terms, so we need to clear the
   * entries because the number of terms in the composite series may have changed.
   */
  clearBasisEvaluation(_number_of_terms);
}

void
CompositeSeriesBasisInterface::setOrder(const std::vector<std::size_t> & orders)
{
  // One order must be specified for each single series
  if (orders.size() != _series.size())
    mooseError(_who_is_using_me,
               " calling CSBI::setOrder(): Mismatch between the orders provided and the number of "
               "series in the functional basis!");

  // Update the orders of each of the single series
  for (std::size_t i = 0; i < _series.size(); ++i)
    _series[i]->setOrder({orders[i]});

  /*
   * After changing the order of each single series, we need to recompute the number of terms by
   * looping over those single series. This also clears the basis evaluation of the composite
   * series.
   */
  setNumberOfTerms();
}

void
CompositeSeriesBasisInterface::setLocation(const Point & point)
{
  // Return if this point is the same as the last at which the composite series was evaluated
  if (point.absolute_fuzzy_equals(_previous_point))
    return;

  // Set the location of each of the single series
  for (auto & series : _series)
    series->setLocation(point);

  // Store the previous point
  _previous_point = point;
}

CompositeSeriesBasisInterface::~CompositeSeriesBasisInterface() {}
