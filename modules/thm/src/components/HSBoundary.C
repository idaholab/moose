#include "HSBoundary.h"
#include "HeatStructureBase.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<HSBoundary>()
{
  InputParameters params = validParams<BoundaryBase>();

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "List of boundary names for which this component applies");
  params.addRequiredParam<std::string>("hs", "Heat structure name");

  return params;
}

HSBoundary::HSBoundary(const InputParameters & params)
  : BoundaryBase(params),

    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _hs_name(getParam<std::string>("hs")),
    _hs_sides(extractHeatStructureSides(_boundary))
{
}

std::vector<HeatStructureBase::SideType>
HSBoundary::extractHeatStructureSides(const std::vector<BoundaryName> & boundary_names) const
{
  std::vector<HeatStructureBase::SideType> hs_sides;

  for (const auto & boundary_name : boundary_names)
  {
    const std::vector<std::string> fields = MooseUtils::split(boundary_name, ":");
    const std::string & hs_side_str = MooseUtils::toUpper(fields[fields.size() - 1]);
    const HeatStructureBase::SideType hs_side =
        THM::stringToEnum<HeatStructureBase::SideType>(hs_side_str);
    hs_sides.push_back(hs_side);
  }

  return hs_sides;
}

void
HSBoundary::check() const
{
  checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_name);

  if (hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);

    // Check that no perimeter is zero; if so, there is not physically a boundary
    for (unsigned int i = 0; i < _boundary.size(); i++)
      if (MooseUtils::absoluteFuzzyEqual(hs.getUnitPerimeter(_hs_sides[i]), 0))
        logError("The heat structure side of the heat structure '",
                 _hs_name,
                 "' corresponding to the boundary name '",
                 _boundary[i],
                 "' has a zero perimeter.");
  }
}
