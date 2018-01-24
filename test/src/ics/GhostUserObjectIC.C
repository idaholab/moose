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
#include "GhostUserObjectIC.h"
#include "GhostUserObject.h"

template <>
InputParameters
validParams<GhostUserObjectIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ghost_uo",
                                          "The GhostUserObject to be coupled into this IC");
  return params;
}

GhostUserObjectIC::GhostUserObjectIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _ghost_uo(getUserObject<GhostUserObject>("ghost_uo"))
{
}

Real
GhostUserObjectIC::value(const Point & /*p*/)
{
  if (_current_elem == NULL)
    return -1.0;

  return _ghost_uo.getElementalValue(_current_elem->id());
}
