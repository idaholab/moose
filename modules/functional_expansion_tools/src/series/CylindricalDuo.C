//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Module includes
#include "CylindricalDuo.h"
#include "Legendre.h"
#include "Zernike.h"

CylindricalDuo::CylindricalDuo() : CompositeSeriesBasisInterface()
{
  // Nothing here, we only needed to call the parent constructor
}

CylindricalDuo::CylindricalDuo(const std::vector<MooseEnum> & domains,
                               const std::vector<std::size_t> & orders,
                               const std::vector<MooseEnum> & series_types)
  : CompositeSeriesBasisInterface(orders, series_types)
{
  // Initialize the pointer to the axial series
  if (_series_types[0] == "Legendre")
    _series.push_back(new Legendre({domains[0]}, {orders[0]}));
  else
    mooseError("CylindricalDuo: No other linear series implemented except Legendre!");

  // Initialize the pointer to the disc series
  if (_series_types[1] == "Zernike")
    _series.push_back(new Zernike({domains[1], domains[2]}, {orders[1]}));
  else
    mooseError("CylindricalDuo: No other disc series implemented except Zernike!");

  /*
   * Set the _number_of_terms for the composite series by looping over each of the single series.
   * This also initializes _basis_evaluation with zero values and the appropriate length.
   */
  setNumberOfTerms();
}

void
CylindricalDuo::setPhysicalBounds(const std::vector<Real> & bounds)
{
  // The axial direction will have two bounds, the disc three
  if (bounds.size() != 5)
    mooseError("CylindricalDuo: Must provide 3 physical bounds: axial_min axial_max disc_center1 "
               "disc_center2 radius");

  _series[0]->setPhysicalBounds({bounds[0], bounds[1]});
  _series[1]->setPhysicalBounds({bounds[2], bounds[3], bounds[4]});
}
