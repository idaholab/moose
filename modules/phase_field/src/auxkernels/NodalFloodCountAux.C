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
  params.addParam<unsigned int>("map_index", "The index of which map to retrieve values from when using NodalFloodCount with multiple maps.");
  return params;
}

NodalFloodCountAux::NodalFloodCountAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _flood_counter(getUserObject<NodalFloodCount>("bubble_object")),
    _var_idx(isParamValid("map_index") ? getParam<unsigned int>("map_index") : 0)
{
}

Real
NodalFloodCountAux::computeValue()
{
  return _flood_counter.getNodeValue(_current_node->id(), _var_idx);
}
