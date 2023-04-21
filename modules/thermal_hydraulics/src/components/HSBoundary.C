//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundary.h"
#include "HeatStructureBase.h"
#include "MooseUtils.h"

InputParameters
HSBoundary::validParams()
{
  InputParameters params = BoundaryBase::validParams();

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "List of boundary names for which this component applies");
  params.addRequiredParam<std::string>("hs", "Heat structure name");

  return params;
}

HSBoundary::HSBoundary(const InputParameters & params)
  : BoundaryBase(params),

    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _hs_name(getParam<std::string>("hs"))
{
}

void
HSBoundary::check() const
{
  checkComponentOfTypeExistsByName<HeatStructureInterface>(_hs_name);

  if (hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);

    // Check that no perimeter is zero; if so, there is not physically a boundary
    for (unsigned int i = 0; i < _boundary.size(); i++)
    {
      if (hs.hasExternalBoundary(_boundary[i]))
      {
        auto hs_side = hs.getExternalBoundaryType(_boundary[i]);
        if ((hs_side == Component2D::ExternalBoundaryType::INNER) ||
            (hs_side == Component2D::ExternalBoundaryType::OUTER))
        {
          if (MooseUtils::absoluteFuzzyEqual(hs.getUnitPerimeter(hs_side), 0))
            logError("The heat structure side of the heat structure '",
                     _hs_name,
                     "' corresponding to the boundary name '",
                     _boundary[i],
                     "' has a zero perimeter. This can be caused by applying the boundary on the "
                     "axis of symmetry of a cylindrical heat structure.");
        }
      }
    }
  }
}
