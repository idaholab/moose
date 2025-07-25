//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<MooseEnum>(
      "hs_side", Component2D::getExternalBoundaryTypeMooseEnum(), "Heat structure side");
  params.addParam<BoundaryName>("hs_boundary", "Name of the coupled heat structure boundary.");

  return params;
}

HSBoundaryInterface::HSBoundaryInterface(Component * component)
  : _hs_name(component->getParam<std::string>("hs")), _hs_side_valid(false)
{
  component->addDependency(_hs_name);

  component->checkMutuallyExclusiveParameters({"hs_side", "hs_boundary"});
}

void
HSBoundaryInterface::setupMesh(const Component * const component)
{
  if (!component->hasComponentByName<HeatStructureBase>(_hs_name))
    return;

  const HeatStructureBase & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

  if (component->isParamValid("hs_side"))
  {
    _hs_side = component->getEnumParam<Component2D::ExternalBoundaryType>("hs_side");
    _hs_side_valid = static_cast<int>(_hs_side) >= 0;
    if (_hs_side_valid)
      _hs_boundary = hs.getExternalBoundaryName(_hs_side);
  }

  if (component->isParamValid("hs_boundary"))
  {
    _hs_boundary = component->getParam<BoundaryName>("hs_boundary");
    _hs_side = hs.getExternalBoundaryType(_hs_boundary);
    _hs_side_valid = static_cast<int>(_hs_side) >= 0;
  }
}

void
HSBoundaryInterface::check(const Component * const component) const
{
  component->checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_name);

  if (component->hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

    if (!hs.hasBoundary(_hs_boundary))
      component->logError("The heat structure boundary '", _hs_boundary, "' does not exist.");

    if (!_hs_side_valid)
      return;

    const Real & P_hs = hs.getUnitPerimeter(_hs_side);
    if (MooseUtils::absoluteFuzzyEqual(P_hs, 0.))
      component->logError("The specified boundary '",
                          _hs_boundary,
                          "' of the heat structure '",
                          _hs_name,
                          "' has a radius of zero.");
    if (std::isnan(P_hs))
      component->logError("The specified boundary '",
                          _hs_boundary,
                          "' of the heat structure '",
                          _hs_name,
                          "' is either START and END boundary, which may not be used.");
  }
}

BoundaryName
HSBoundaryInterface::getHSBoundaryName(const Component * const /*component*/) const
{
  return _hs_boundary;
}

Component2D::ExternalBoundaryType
HSBoundaryInterface::getHSExternalBoundaryType() const
{
  return _hs_side;
}
