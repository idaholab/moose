//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSBoundaryInterface.h"
#include "Component.h"
#include "THMMesh.h"

InputParameters
HSBoundaryInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<std::string>("hs", "Heat structure name");
  params.addRequiredParam<MooseEnum>(
      "hs_side", Component2D::getExternalBoundaryTypeMooseEnum(), "Heat structure side");

  return params;
}

HSBoundaryInterface::HSBoundaryInterface(Component * component)
  : _hs_name(component->getParam<std::string>("hs")),
    _hs_side_enum(component->getParam<MooseEnum>("hs_side")),
    _hs_side(component->getEnumParam<Component2D::ExternalBoundaryType>("hs_side")),
    _hs_side_valid(static_cast<int>(_hs_side) >= 0)
{
  component->addDependency(_hs_name);
}

void
HSBoundaryInterface::check(const Component * const component) const
{
  component->checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_name);

  if (component->hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

    if (_hs_side_valid)
    {
      const Real & P_hs = hs.getUnitPerimeter(_hs_side);
      if (MooseUtils::absoluteFuzzyEqual(P_hs, 0.))
        component->logError("'hs_side' parameter is set to '",
                            _hs_side_enum,
                            "', but this side of the heat structure '",
                            _hs_name,
                            "' has radius of zero.");

      if (std::isnan(P_hs))
        component->logError("Invalid side '",
                            _hs_side_enum,
                            "'. This side does not have unit perimeter. You probably want to use "
                            "'INNER' or 'OUTER' side instead.");
    }
  }
}

const BoundaryName &
HSBoundaryInterface::getHSBoundaryName(const Component * const component) const
{
  const HeatStructureBase & hs = component->getComponentByName<HeatStructureBase>(_hs_name);
  return hs.getExternalBoundaryName(_hs_side);
}
