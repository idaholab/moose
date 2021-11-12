//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Cartesian.h"
#include "Legendre.h"

#include <memory>

Cartesian::Cartesian(const std::string & who_is_using_me)
  : CompositeSeriesBasisInterface(who_is_using_me)
{
}

Cartesian::Cartesian(const std::vector<MooseEnum> & domains,
                     const std::vector<std::size_t> & orders,
                     const std::vector<MooseEnum> & series_types,
                     const std::string & who_is_using_me,
                     MooseEnum expansion_type,
                     MooseEnum generation_type)
  : CompositeSeriesBasisInterface(orders, series_types, who_is_using_me)
{
  // Initialize the pointers to each of the single series
  for (std::size_t i = 0; i < _series_types.size(); ++i)
    if (_series_types[i] == "Legendre")
    {
      std::vector<MooseEnum> local_domain = {domains[i]};
      std::vector<std::size_t> local_order = {orders[i]};
      _series.push_back(
          std::make_unique<Legendre>(local_domain, local_order, expansion_type, generation_type));
    }
    else
      mooseError("Cartesian: No other linear series implemented except Legendre!");

  /*
   * Set the _number_of_terms for the composite series by looping over each of the single series.
   * This also initializes _basis_evaluation with zero values and the appropriate length.
   */
  setNumberOfTerms();
}

void
Cartesian::setPhysicalBounds(const std::vector<Real> & bounds)
{
  // Each single series is assumed to be a function of a single variable so that it has two bounds
  if (bounds.size() != _series_types.size() * 2)
    mooseError("Cartesian: Mismatch between the physical bounds provided and the number of series "
               "in the functional basis!");

  // Update the _physical_bounds of each of the single series
  unsigned int j = 0;
  for (std::size_t i = 0; i < _series_types.size(); ++i, j += 2)
    _series[i]->setPhysicalBounds({bounds[j], bounds[j + 1]});
}
