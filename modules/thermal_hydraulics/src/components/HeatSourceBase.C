#include "HeatSourceBase.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindricalBase.h"
#include "HeatStructurePlate.h"
#include "HeatStructureFromFile3D.h"

InputParameters
HeatSourceBase::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<std::string>("hs", "Heat structure in which to apply heat source");
  params.addRequiredParam<std::vector<std::string>>(
      "regions", "Heat structure regions where heat generation is to be applied");
  params.addClassDescription("Base class for heat source components");
  return params;
}

HeatSourceBase::HeatSourceBase(const InputParameters & parameters)
  : Component(parameters), _region_names(getParam<std::vector<std::string>>("regions"))
{
  checkSizeGreaterThan<std::string>("regions", 0);
}

void
HeatSourceBase::check() const
{
  Component::check();

  checkComponentOfTypeExists<HeatStructureBase>("hs");

  if (hasComponent<HeatStructureBase>("hs"))
  {
    if (hasComponent<HeatStructurePlate>("hs") || hasComponent<HeatStructureCylindricalBase>("hs"))
    {
      const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
      for (auto && region : _region_names)
        if (!hs.hasBlock(region))
          logError("Region '",
                   region,
                   "' does not exist in heat structure '",
                   getParam<std::string>("hs"),
                   "'.");
    }
    else if (hasComponent<HeatStructureFromFile3D>("hs"))
    {
      const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
      for (auto && region : _region_names)
        if (!hs.hasBlock(region))
          logError("Region '",
                   region,
                   "' does not exist in heat structure '",
                   getParam<std::string>("hs"),
                   "'.");
    }
    else
      logError("Heat structure must be of type 'HeatStructurePlate', "
               "'HeatStructureCylindricalBase' or 'HeatStructureFromFile3D'.");
  }
}
