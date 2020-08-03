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

HSBoundaryInterface::HSBoundaryInterface(const Component * const component)
  : _hs_name(component->getParam<std::string>("hs")),
    _hs_side_enum(component->getParam<MooseEnum>("hs_side")),
    _hs_side(THM::stringToEnum<HeatStructureBase::SideType>(_hs_side_enum))
{
  if (_hs_side == HeatStructureBase::OUTER || _hs_side == HeatStructureBase::INNER ||
      _hs_side == HeatStructureBase::START || _hs_side == HeatStructureBase::END)
    _hs_side_valid = true;
  else
    _hs_side_valid = false;
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
    case HeatStructureBase::OUTER:
      if (hs.getOuterBoundaryNames().size() > 0)
        return hs.getOuterBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case HeatStructureBase::INNER:
      if (hs.getInnerBoundaryNames().size() > 0)
        return hs.getInnerBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case HeatStructureBase::START:
      if (hs.getStartBoundaryNames().size() > 0)
        return hs.getStartBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case HeatStructureBase::END:
      if (hs.getEndBoundaryNames().size() > 0)
        return hs.getEndBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;
  }
}
