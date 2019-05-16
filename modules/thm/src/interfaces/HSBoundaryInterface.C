#include "HSBoundaryInterface.h"
#include "Component.h"
#include "HeatStructureBase.h"
#include "THMMesh.h"

template <>
InputParameters
validParams<HSBoundaryInterface>()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<std::string>("hs", "Heat structure name");
  MooseEnum hs_sides("top bottom");
  params.addRequiredParam<MooseEnum>("hs_side", hs_sides, "Heat structure side");

  return params;
}

HSBoundaryInterface::HSBoundaryInterface(const Component * const component)
  : _hs_name(component->getParam<std::string>("hs")),
    _hs_side(component->getParam<MooseEnum>("hs_side"))
{
}

void
HSBoundaryInterface::check(const Component * const component) const
{
  component->checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_name);

  if (component->hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

    const Real & P_hs = hs.getUnitPerimeter(_hs_side);
    if (MooseUtils::absoluteFuzzyEqual(P_hs, 0.))
      component->logError("'hs_side' parameter is set to '",
                          _hs_side,
                          "', but this side of the heat structure '",
                          _hs_name,
                          "' has radius of zero.");
  }
}

const BoundaryName &
HSBoundaryInterface::getHSBoundaryName(const Component * const component) const
{
  const HeatStructureBase & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

  switch (_hs_side)
  {
    case 0:
      if (hs.getTopBoundaryNames().size() > 0)
        return hs.getTopBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case 1:
      if (hs.getBottomBoundaryNames().size() > 0)
        return hs.getBottomBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    default:
      mooseError(component->name(), ": Unknown side specified in the 'hs_side' parameter.");
  }
}
