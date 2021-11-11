//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CylindricalDuo.h"
#include "Legendre.h"
#include "Zernike.h"

#include <memory>

CylindricalDuo::CylindricalDuo(const std::string & who_is_using_me)
  : CompositeSeriesBasisInterface(who_is_using_me)
{
}

CylindricalDuo::CylindricalDuo(const std::vector<MooseEnum> & domains,
                               const std::vector<std::size_t> & orders,
                               const std::vector<MooseEnum> & series_types,
                               const std::string & who_is_using_me,
                               MooseEnum expansion_type,
                               MooseEnum generation_type)
  : CompositeSeriesBasisInterface(orders, series_types, who_is_using_me)
{
  // Initialize the pointer to the axial series
  if (_series_types[0] == "Legendre")
  {
    std::vector<MooseEnum> local_domain = {domains[0]};
    std::vector<std::size_t> local_order = {orders[0]};
    _series.push_back(
        std::make_unique<Legendre>(local_domain, local_order, expansion_type, generation_type));
  }
  else
    mooseError("CylindricalDuo: No other linear series implemented except Legendre!");

  // Initialize the pointer to the disc series
  if (_series_types[1] == "Zernike")
  {
    std::vector<MooseEnum> local_domain = {domains[1], domains[2]};
    std::vector<std::size_t> local_order = {orders[1]};
    _series.push_back(
        std::make_unique<Zernike>(local_domain, local_order, expansion_type, generation_type));
  }
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
