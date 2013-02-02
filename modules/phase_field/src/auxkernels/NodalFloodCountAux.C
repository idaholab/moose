/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalFloodCountAux.h"
#include "NodalFloodCount.h"
#include "MooseEnum.h"

template<>
InputParameters validParams<NodalFloodCountAux>()
{
  MooseEnum field_display("UNIQUE_REGION, VARIABLE_COLORING, CENTROID", "UNIQUE_REGION");
  
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
    _var_coloring(false)
{
  if (isNodal())
  {
    if (_field_display == "CENTROID")
      mooseError("CENTROID coloring is only available for elemental aux variables");
    else if (_field_display == "VARIABLE_COLORING")
      _var_coloring = true;
  }
  else if (_field_display != "CENTROID")
    mooseError("UNIQUE_REGION and VARIABLE_COLORING is only avaialble for nodal aux variables");
}

Real
NodalFloodCountAux::computeValue()
{
  if (isNodal())
    return _flood_counter.getNodalValue(_current_node->id(), _var_idx, _var_coloring);
  else
    return _flood_counter.getElementalValue(_current_elem->id());
}
