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

#include "GhostAux.h"
#include "GhostUserObject.h"

template <>
InputParameters
validParams<GhostAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<UserObjectName>("ghost_user_object",
                                  "The GhostUserObject where this Aux pulls values from");
  MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_BEGIN});
  params.addClassDescription("Aux Kernel to display ghosted elements from a single processor or "
                             "the union on all processors");
  return params;
}

GhostAux::GhostAux(const InputParameters & params)
  : AuxKernel(params), _ghost_uo(getUserObject<GhostUserObject>("ghost_user_object"))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
GhostAux::computeValue()
{
  return _ghost_uo.getElementalValue(_current_elem->id());
}
