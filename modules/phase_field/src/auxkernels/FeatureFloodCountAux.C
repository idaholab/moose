/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FeatureFloodCountAux.h"
#include "FeatureFloodCount.h"
#include "MooseEnum.h"

template<>
InputParameters validParams<FeatureFloodCountAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Feature detection by connectivity analysis");
  params.addRequiredParam<UserObjectName>("bubble_object", "The FeatureFloodCount UserObject to get values from.");
  params.addParam<unsigned int>("map_index", "The index of which map to retrieve values from when using FeatureFloodCount with multiple maps.");
  MooseEnum field_display("UNIQUE_REGION VARIABLE_COLORING GHOSTED_ENTITIES HALOS ACTIVE_BOUNDS", "UNIQUE_REGION");
  params.addParam<MooseEnum>("field_display", field_display, "Determines how the auxilary field should be colored. (UNIQUE_REGION and VARIABLE_COLORING are nodal, CENTROID is elemental, default: UNIQUE_REGION)");

  MultiMooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "initial timestep_end";
  params.set<MultiMooseEnum>("execute_on") = execute_options;

  return params;
}

FeatureFloodCountAux::FeatureFloodCountAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _flood_counter(getUserObject<FeatureFloodCount>("bubble_object")),
    _var_idx(isParamValid("map_index") ? getParam<unsigned int>("map_index") : std::numeric_limits<unsigned int>::max()),
    _field_display(getParam<MooseEnum>("field_display")),
    _var_coloring(_field_display == "VARIABLE_COLORING"),
    _field_type(static_cast<FeatureFloodCount::FieldType>(static_cast<int>(_field_display)))
{
  if (_flood_counter.isElemental() == isNodal() &&
      (_field_display == "UNIQUE_REGION" || _field_display == "VARIABLE_COLORING" || _field_display == "GHOSTED_ENTITIES" || _field_display == "HALOS"))
    mooseError("UNIQUE_REGION, VARIABLE_COLORING, GHOSTED_ENTITITES and HALOS must be on variable types that match the entity mode of the FeatureFloodCounter");

  if (isNodal() && _field_display == "ACTIVE_BOUNDS")
    mooseError("ACTIVE_BOUNDS is only available for elemental aux variables");
}

Real
FeatureFloodCountAux::computeValue()
{
  switch (_field_display)
  {
  case 0:  // UNIQUE_REGION
  case 1:  // VARIABLE_COLORING
  case 2:  // GHOSTED_ENTITIES
  case 3:  // HALOS
    return _flood_counter.getEntityValue((isNodal() ? _current_node->id() : _current_elem->id()), _field_type, _var_idx);
  case 4:  // ACTIVE_BOUNDS
    return _flood_counter.getElementalValues(_current_elem->id()).size();

  default:
    mooseError("Unimplemented \"field_display\" type");
  }

  return 0;
}
