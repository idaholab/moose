//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingleSeriesBasisInterface.h"

#include "libmesh/point.h"

/*
 * The default constructor for a single series creates a single-term functional basis of zeroth
 * order. Although the _physical_bounds flag is set to false anyways, we need to assign some value
 * here in the constructor so that the order of member variables in the include file doesn't give an
 * error. The same holds for _standardized_location.
 */
SingleSeriesBasisInterface::SingleSeriesBasisInterface()
  : FunctionalBasisInterface(1),
    _domains({_domain_options = "x"}),
    _orders({0}),
    _physical_bounds(2, 0.0),
    _standardized_location({0.0}),
    _are_physical_bounds_specified(false),
    _location(_domains.size(), 0.0)
{
}

SingleSeriesBasisInterface::SingleSeriesBasisInterface(const std::vector<MooseEnum> & domains,
                                                       const std::vector<std::size_t> & orders,
                                                       const unsigned int number_of_terms)
  : FunctionalBasisInterface(number_of_terms),
    _domains(domains),
    _orders(orders),
    _physical_bounds(2, 0.0),
    _standardized_location(1, 0.0),
    _are_physical_bounds_specified(false),
    _location(_domains.size(), 0.0)
{
}

bool
SingleSeriesBasisInterface::isCacheInvalid() const
{
  return _is_cache_invalid;
}

void
SingleSeriesBasisInterface::evaluateGeneration()
{
  _evaluateGenerationWrapper();
}

void
SingleSeriesBasisInterface::evaluateExpansion()
{
  _evaluateExpansionWrapper();
}

void
SingleSeriesBasisInterface::setOrder(const std::vector<std::size_t> & orders)
{
  if (orders.size() != _orders.size())
    mooseError("SSBI: Invalid 'orders' use in setOrder()!");

  /*
   * Do nothing if the order isn't changed. Note that this only compares the first value - it is
   * assumed that a single series only needs to be described using a single order.
   */
  if (orders[0] == _orders[0])
    return;

  _orders = orders;

  // Set the new number of terms in the single series
  _number_of_terms = calculatedNumberOfTermsBasedOnOrder(_orders);

  // Zero the basis evaluation
  clearBasisEvaluation(_number_of_terms);
  _is_cache_invalid = true;
}

void
SingleSeriesBasisInterface::setPhysicalBounds(const std::vector<Real> & bounds)
{
  // Use the concrete implementation to check the validity of the bounds
  checkPhysicalBounds(bounds);

  _physical_bounds = bounds;
  _are_physical_bounds_specified = true;

  /*
   * Once the physical bounds have been changed, the normalization of a point will change, so the
   * cached values will also be incorrect
   */
  _is_cache_invalid = true;
}

void
SingleSeriesBasisInterface::setLocation(const Point & point)
{
  std::vector<Real> oldLocation(_location);

  // Update the physical-space location
  _location = extractLocationFromPoint(point);

  // Standardize the location if standardized bounds exist
  if (_are_physical_bounds_specified)
    _standardized_location = getStandardizedLocation(_location);
  else
    _standardized_location = _location;

  // Once the location is changed, the cached values correspond to an old location
  for (std::size_t i = 0; !_is_cache_invalid && (i < _location.size()); ++i)
    if (oldLocation[i] != _location[i])
      _is_cache_invalid = true;
}

std::vector<Real>
SingleSeriesBasisInterface::extractLocationFromPoint(const Point & point) const
{
  std::vector<Real> location(_domains.size());

  // Update the locations as specified by _domain
  for (std::size_t index = 0; index < _domains.size(); ++index)
    location[index] = point(_domains[index]);
  return location;
}

std::size_t
SingleSeriesBasisInterface::getOrder(std::size_t domain) const
{
  return domain < _orders.size() ? _orders[domain] : -1;
}

SingleSeriesBasisInterface::~SingleSeriesBasisInterface() {}
