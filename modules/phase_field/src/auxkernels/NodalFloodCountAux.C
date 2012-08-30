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
  return params;
}

NodalFloodCountAux::NodalFloodCountAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _flood_counter(getUserObject<NodalFloodCount>("bubble_object"))
{
}

Real
NodalFloodCountAux::computeValue()
{
  return _flood_counter.getNodeValue(_current_node->id());
}
