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

template<>
InputParameters validParams<NodalFloodCountAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("bubble_object", "The NodalFloodCount UserObject to get values from.");
  params.addParam<unsigned int>("map_index", 0, "The index of which map to retrieve values from when using NodalFloodCount with multiple maps.");
  params.addParam<bool>("show_var_coloring", false, "Display the variable index instead of the unique bubble id.");
  return params;
}

NodalFloodCountAux::NodalFloodCountAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _flood_counter(getUserObject<NodalFloodCount>("bubble_object")),
    _var_idx(getParam<unsigned int>("map_index")),
    _var_coloring(getParam<bool>("show_var_coloring"))
{
}

Real
NodalFloodCountAux::computeValue()
{
  return _flood_counter.getNodeValue(_current_node->id(), _var_idx, _var_coloring);
}
