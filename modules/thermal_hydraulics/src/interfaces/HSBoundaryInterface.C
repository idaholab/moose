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
  : _hs_name(component->getParam<std::string>("hs")),
    _hs_side(component->getEnumParam<Component2D::ExternalBoundaryType>("hs_side", false)),
    _hs_side_valid(static_cast<int>(_hs_side) >= 0)
{
  component->addDependency(_hs_name);

  component->checkMutuallyExclusiveParameters({"hs_side", "hs_boundary"});
}

void
HSBoundaryInterface::check(const Component * const component) const
{
  component->checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_name);

  if (component->hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const auto & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

    BoundaryName hs_boundary;
    Component2D::ExternalBoundaryType hs_side;
    if (component->isParamValid("hs_side"))
    {
      if (_hs_side_valid)
      {
        hs_boundary = hs.getExternalBoundaryName(_hs_side);
        hs_side = _hs_side;
      }
      else
      {
        component->logError("The parameter 'hs_side' was given an invalid value.");
        return;
      }
    }
    else
    {
      hs_boundary = component->getParam<BoundaryName>("hs_boundary");
      hs_side = hs.getExternalBoundaryType(hs_boundary);
    }

    if (!hs.hasBoundary(hs_boundary))
      component->logError("The heat structure boundary '", hs_boundary, "' does not exist.");

    const Real & P_hs = hs.getUnitPerimeter(hs_side);
    if (MooseUtils::absoluteFuzzyEqual(P_hs, 0.))
    {
      if (component->isParamValid("hs_side"))
        component->logError("'hs_side' parameter is set to '",
                            component->getParam<MooseEnum>("hs_side"),
                            "', but this side of the heat structure '",
                            _hs_name,
                            "' has radius of zero.");
      else
        component->logError("The specified boundary '",
                            hs_boundary,
                            "' of the heat structure '",
                            _hs_name,
                            "' has a radius of zero.");
    }
    if (std::isnan(P_hs))
      component->logError("The specified boundary '",
                          hs_boundary,
                          "' of the heat structure '",
                          _hs_name,
                          "' is either START and END boundary, which may not be used.");
  }
}

const BoundaryName &
HSBoundaryInterface::getHSBoundaryName(const Component * const component) const
{
  if (component->isParamValid("hs_side"))
  {
    const auto & hs = component->getComponentByName<HeatStructureBase>(_hs_name);
    return hs.getExternalBoundaryName(_hs_side);
  }
  else
    return component->getParam<BoundaryName>("hs_boundary");
}

Component2D::ExternalBoundaryType
HSBoundaryInterface::getHSExternalBoundaryType(const Component * const component) const
{
  if (component->isParamValid("hs_side"))
    return _hs_side;
  else
  {
    const auto & hs = component->getComponentByName<HeatStructureBase>(_hs_name);
    const auto hs_boundary = component->getParam<BoundaryName>("hs_boundary");
    return hs.getExternalBoundaryType(hs_boundary);
  }
}

bool
HSBoundaryInterface::HSBoundaryIsValid(const Component * const component) const
{
  const auto & hs = component->getComponentByName<HeatStructureBase>(_hs_name);

  BoundaryName hs_boundary;
  if (component->isParamValid("hs_side"))
  {
    if (_hs_side_valid)
      hs_boundary = hs.getExternalBoundaryName(_hs_side);
    else
      return false;
  }
  else
    hs_boundary = component->getParam<BoundaryName>("hs_boundary");

  return hs.hasBoundary(hs_boundary);
}
