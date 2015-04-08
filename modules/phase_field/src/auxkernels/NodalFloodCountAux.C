/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "NodalFloodCountAux.h"
#include "NodalFloodCount.h"
#include "MooseEnum.h"

template<>
InputParameters validParams<NodalFloodCountAux>()
{
  MooseEnum field_display("UNIQUE_REGION VARIABLE_COLORING ACTIVE_BOUNDS CENTROID", "UNIQUE_REGION");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("bubble_object", "The NodalFloodCount UserObject to get values from.");
  params.addParam<unsigned int>("map_index", 0, "The index of which map to retrieve values from when using NodalFloodCount with multiple maps.");
  params.addParam<MooseEnum>("field_display", field_display, "Determines how the auxilary field should be colored. (UNIQUE_REGION and VARIABLE_COLORING are nodal, CENTROID is elemental, default: UNIQUE_REGION)");
  return params;
}

NodalFloodCountAux::NodalFloodCountAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _flood_counter(getUserObject<NodalFloodCount>("bubble_object")),
    _var_idx(getParam<unsigned int>("map_index")),
    _field_display(getParam<MooseEnum>("field_display")),
    _var_coloring(_field_display == "VARIABLE_COLORING")
{
  if (isNodal())
  {
    if (_field_display == "CENTROID")
      mooseError("CENTROID coloring is only available for elemental aux variables");

    if (_field_display == "ACTIVE_BOUNDS")
      mooseError("ACTIVE_BOUNDS is only available for elemental aux variables");

    if (_flood_counter.isElemental() && (_field_display == "UNIQUE_REGION" || _field_display == "VARIABLE_COLORING"))
      mooseError("UNIQUE_REGION and VARIABLE_COLORING must be on variable types that match the entity mode of the NodalFloodCounter");
  }
  else
  {
    if (! _flood_counter.isElemental() && (_field_display == "UNIQUE_REGION" || _field_display == "VARIABLE_COLORING"))
      mooseError("UNIQUE_REGION and VARIABLE_COLORING must be on variable types that match the entity mode of the NodalFloodCounter");
  }
}

Real
NodalFloodCountAux::computeValue()
{
  switch (_field_display)
  {
  case 0:  // UNIQUE_REGION
  case 1:  // VARIABLE_COLORING
    return _flood_counter.getEntityValue((isNodal() ? _current_node->id() : _current_elem->id()), _var_idx, _var_coloring);
  case 2:  // ACTIVE_BOUNDS
    return _flood_counter.getElementalValues(_current_elem->id()).size();
  case 3:  // CENTROID
    return _flood_counter.getElementalValue(_current_elem->id());
  }

  return 0;
}
