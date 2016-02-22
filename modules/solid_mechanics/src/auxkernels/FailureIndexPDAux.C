/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FailureIndexPDAux.h"

template<>
InputParameters validParams<FailureIndexPDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("failure_index_pd","The name of the FailureIndexPD user object");
  return params;
}

FailureIndexPDAux::FailureIndexPDAux(const InputParameters & parameters) :
  AuxKernel(parameters),
  _failure_index_pd(&getUserObject<FailureIndexPD>("failure_index_pd"))
{
}

Real
FailureIndexPDAux::computeValue()
{
  if (!isNodal())
    mooseError("must run on a nodal variable");

  return _failure_index_pd->computeFailureIndex(_current_node->id());
}
