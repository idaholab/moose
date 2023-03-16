//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSourceBase.h"
#include "HeatStructureBase.h"
#include "HeatStructureInterface.h"
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
  : Component(parameters),
    _hs_name(getParam<std::string>("hs")),
    _region_names(getParam<std::vector<std::string>>("regions"))
{
  checkSizeGreaterThan<std::string>("regions", 0);

  for (auto && region : _region_names)
    _subdomain_names.push_back(genName(_hs_name, region));
}

void
HeatSourceBase::check() const
{
  Component::check();

  checkComponentOfTypeExists<HeatStructureInterface>("hs");

  if (hasComponent<HeatStructureBase>("hs"))
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
    const HeatStructureFromFile3D & hs = getComponent<HeatStructureFromFile3D>("hs");
    for (auto && region : _region_names)
      if (!hs.hasRegion(region))
        logError("Region '",
                 region,
                 "' does not exist in heat structure '",
                 getParam<std::string>("hs"),
                 "'.");
  }
  else
    logError("Heat structure must be of type 'HeatStructureBase' or 'HeatStructureFromFile3D'.");
}
