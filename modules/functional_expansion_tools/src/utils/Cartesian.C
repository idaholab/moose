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

#include "Cartesian.h"
#include "Legendre.h"

Cartesian::Cartesian() : CompositeSeriesBasisInterface()
{
  // Nothing here, we only needed to call the parent constructor
}

Cartesian::Cartesian(const std::vector<MooseEnum> & domains,
                     const std::vector<std::size_t> & orders,
                     const std::vector<MooseEnum> & series_types)
  : CompositeSeriesBasisInterface(orders, series_types)
{
  // Initialize the pointers to each of the single series
  for (std::size_t i = 0; i < _series_types.size(); ++i)
    if (_series_types[i] == "Legendre")
      _series.push_back(new Legendre({domains[i]}, {orders[i]}));
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
