//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NeighborMooseVariableInterface.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseTypes.h"
#include "Problem.h"
#include "SubProblem.h"

NeighborMooseVariableInterface::NeighborMooseVariableInterface(const MooseObject * moose_object,
                                                               bool nodal)
  : MooseVariableInterface(moose_object, nodal)
{
}

NeighborMooseVariableInterface::~NeighborMooseVariableInterface() {}

const VariableValue &
NeighborMooseVariableInterface::neighborValue()
{
  if (_nodal)
    return _variable->nodalSlnNeighbor();
  else
    return _variable->slnNeighbor();
}

const VariableValue &
NeighborMooseVariableInterface::neighborValueOld()
{
  if (_nodal)
    return _variable->nodalSlnOldNeighbor();
  else
    return _variable->slnOldNeighbor();
}

const VariableValue &
NeighborMooseVariableInterface::neighborValueOlder()
{
  if (_nodal)
    return _variable->nodalSlnOlderNeighbor();
  else
    return _variable->slnOlderNeighbor();
}

const VariableGradient &
NeighborMooseVariableInterface::neighborGradient()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnNeighbor();
}

const VariableGradient &
NeighborMooseVariableInterface::neighborGradientOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOldNeighbor();
}

const VariableGradient &
NeighborMooseVariableInterface::neighborGradientOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOlderNeighbor();
}

const VariableSecond &
NeighborMooseVariableInterface::neighborSecond()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnNeighbor();
}

const VariableSecond &
NeighborMooseVariableInterface::neighborSecondOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOldNeighbor();
}

const VariableSecond &
NeighborMooseVariableInterface::neighborSecondOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOlderNeighbor();
}

const VariableTestSecond &
NeighborMooseVariableInterface::neighborSecondTest()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondPhiFaceNeighbor();
}

const VariablePhiSecond &
NeighborMooseVariableInterface::neighborSecondPhi()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _mvi_assembly->secondPhiFaceNeighbor();
}
