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

bool
HSBoundary::allComponent2DBoundariesAreExternal() const
{
  const auto & comp2d = getComponentByName<Component2D>(_hs_name);
  for (const auto & boundary : _boundary)
    if (!comp2d.hasExternalBoundary(boundary))
      return false;
  return true;
}

void
HSBoundary::checkAllComponent2DBoundariesAreExternal() const
{
  if (!allComponent2DBoundariesAreExternal())
    logError("The boundaries given in 'boundary' must all be external.");
}

bool
HSBoundary::hasCommonComponent2DExternalBoundaryType() const
{
  if (!_boundary.empty())
  {
    const auto & comp2d = getComponentByName<Component2D>(_hs_name);
    const auto common_boundary_type = comp2d.getExternalBoundaryType(_boundary[0]);
    for (unsigned int i = 1; i < _boundary.size(); i++)
    {
      const auto boundary_type = comp2d.getExternalBoundaryType(_boundary[i]);
      if (boundary_type != common_boundary_type)
        return false;
    }
    return true;
  }
  mooseError("No boundaries were supplied in 'boundary'.");
}

Component2D::ExternalBoundaryType
HSBoundary::getCommonComponent2DExternalBoundaryType() const
{
  if (hasCommonComponent2DExternalBoundaryType())
  {
    const auto & comp2d = getComponentByName<Component2D>(_hs_name);
    return comp2d.getExternalBoundaryType(_boundary[0]);
  }
  else
    mooseError(
        "The boundaries supplied in 'boundary' do not have a common external boundary type.");
}
