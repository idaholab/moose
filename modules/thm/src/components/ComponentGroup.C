#include "ComponentGroup.h"

registerMooseObject("THMApp", ComponentGroup);

template <>
InputParameters
validParams<ComponentGroup>()
{
  InputParameters params = validParams<THMObject>();
  params.addClassDescription("Group of components. Used only for parsing input files.");
  params.addPrivateParam<std::string>("built_by_action", "add_component");
  params.registerBase("Component");
  return params;
}

ComponentGroup::ComponentGroup(const InputParameters & parameters) : THMObject(parameters) {}
