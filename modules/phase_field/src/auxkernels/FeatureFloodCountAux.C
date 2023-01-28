//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FeatureFloodCountAux.h"
#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"
#include "MooseEnum.h"

#include <algorithm>

registerMooseObject("PhaseFieldApp", FeatureFloodCountAux);

InputParameters
FeatureFloodCountAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Feature detection by connectivity analysis");
  params.addDeprecatedParam<UserObjectName>("bubble_object",
                                            "The FeatureFloodCount UserObject to get values from.",
                                            "Use \"flood_counter\" instead.");
  params.addRequiredParam<UserObjectName>("flood_counter",
                                          "The FeatureFloodCount UserObject to get values from.");
  params.addParam<unsigned int>("map_index",
                                "The index of which map to retrieve values from when "
                                "using FeatureFloodCount with multiple maps.");
  MooseEnum field_display("UNIQUE_REGION VARIABLE_COLORING GHOSTED_ENTITIES HALOS CENTROID "
                          "ACTIVE_BOUNDS INTERSECTS_SPECIFIED_BOUNDARY",
                          "UNIQUE_REGION");
  params.addParam<MooseEnum>("field_display",
                             field_display,
                             "Determines how the auxilary field should be colored. "
                             "(UNIQUE_REGION and VARIABLE_COLORING are nodal, CENTROID is "
                             "elemental, default: UNIQUE_REGION)");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  return params;
}

FeatureFloodCountAux::FeatureFloodCountAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _flood_counter(getUserObject<FeatureFloodCount>("flood_counter")),
    _var_idx(isParamValid("map_index") ? getParam<unsigned int>("map_index")
                                       : std::numeric_limits<std::size_t>::max()),
    _field_type(getParam<MooseEnum>("field_display").getEnum<FieldType>()),
    _var_coloring(_field_type == FeatureFloodCount::FieldType::VARIABLE_COLORING)
{
  if (_flood_counter.isElemental() == isNodal() &&
      (_field_type == FieldType::UNIQUE_REGION || _field_type == FieldType::VARIABLE_COLORING ||
       _field_type == FieldType::GHOSTED_ENTITIES || _field_type == FieldType::HALOS))
    paramError("field_display",
               "UNIQUE_REGION, VARIABLE_COLORING, GHOSTED_ENTITIES and HALOS must be on variable "
               "types that match the entity mode of the FeatureFloodCounter");

  if (isNodal() && (_field_type == FieldType::ACTIVE_BOUNDS || _field_type == FieldType::CENTROID ||
                    _field_type == FieldType::INTERSECTS_SPECIFIED_BOUNDARY))
    paramError("field_display",
               "The selected option is only available for elemental aux variables");
}

void
FeatureFloodCountAux::precalculateValue()
{
  switch (_field_type)
  {
    case FieldType::UNIQUE_REGION:
    case FieldType::VARIABLE_COLORING:
    case FieldType::GHOSTED_ENTITIES:
    case FieldType::HALOS:
    case FieldType::CENTROID:
    case FieldType::INTERSECTS_SPECIFIED_BOUNDARY:
      _value = _flood_counter.getEntityValue(
          (isNodal() ? _current_node->id() : _current_elem->id()), _field_type, _var_idx);
      break;

    case FieldType::ACTIVE_BOUNDS:
    {
      const auto & var_to_features = _flood_counter.getVarToFeatureVector(_current_elem->id());
      _value = std::count_if(var_to_features.begin(),
                             var_to_features.end(),
                             [](unsigned int feature_id)
                             { return feature_id != FeatureFloodCount::invalid_id; });

      break;
    }

    default:
      paramError("field_display", "Unimplemented type");
  }
}

Real
FeatureFloodCountAux::computeValue()
{
  return _value;
}
