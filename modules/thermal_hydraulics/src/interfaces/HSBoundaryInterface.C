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
      "hs_side", HeatStructureBase::getSideType(), "Heat structure side");

  return params;
}

HSBoundaryInterface::HSBoundaryInterface(Component * component)
  : _hs_name(component->getParam<std::string>("hs")),
    _hs_side_enum(component->getParam<MooseEnum>("hs_side")),
    _hs_side(THM::stringToEnum<HeatStructureSideType>(_hs_side_enum))
{
  if (_hs_side == HeatStructureSideType::OUTER || _hs_side == HeatStructureSideType::INNER ||
      _hs_side == HeatStructureSideType::START || _hs_side == HeatStructureSideType::END)
    _hs_side_valid = true;
  else
    _hs_side_valid = false;

  component->addDependency(_hs_name);
}

void
HSBoundaryInterface::check(const Component * const component) const
{
  if (!_hs_side_valid)
    component->logError("Invalid option '",
                        _hs_side_enum,
                        "' provided for 'hs_side' parameter. Valid options (not case-sensitive) "
                        "are 'inner' and 'outer'.");

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

  switch (_hs_side)
  {
    case HeatStructureSideType::OUTER:
      if (hs.getOuterBoundaryNames().size() > 0)
        return hs.getOuterBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case HeatStructureSideType::INNER:
      if (hs.getInnerBoundaryNames().size() > 0)
        return hs.getInnerBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case HeatStructureSideType::START:
      if (hs.getStartBoundaryNames().size() > 0)
        return hs.getStartBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case HeatStructureSideType::END:
      if (hs.getEndBoundaryNames().size() > 0)
        return hs.getEndBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;
  }

  mooseError(component->name(), ": Unknown value of 'hs_side' parameter.");
}
