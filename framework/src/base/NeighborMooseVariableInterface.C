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

#include "NeighborMooseVariableInterface.h"
#include "Problem.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

NeighborMooseVariableInterface::NeighborMooseVariableInterface(const InputParameters & parameters, bool nodal) :
    MooseVariableInterface(parameters, nodal)
{
}

NeighborMooseVariableInterface::~NeighborMooseVariableInterface()
{
}


VariableValue &
NeighborMooseVariableInterface::neighborValue()
{
  if (_nodal)
    return _variable->nodalSlnNeighbor();
  else
    return _variable->slnNeighbor();
}

VariableValue &
NeighborMooseVariableInterface::neighborValueOld()
{
  if (_nodal)
    return _variable->nodalSlnOldNeighbor();
  else
    return _variable->slnOldNeighbor();
}

VariableValue &
NeighborMooseVariableInterface::neighborValueOlder()
{
  if (_nodal)
    return _variable->nodalSlnOlderNeighbor();
  else
    return _variable->slnOlderNeighbor();
}

VariableGradient &
NeighborMooseVariableInterface::neighborGradient()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnNeighbor();
}

VariableGradient &
NeighborMooseVariableInterface::neighborGradientOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOldNeighbor();
}

VariableGradient &
NeighborMooseVariableInterface::neighborGradientOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOlderNeighbor();
}

VariableSecond &
NeighborMooseVariableInterface::neighborSecond()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnNeighbor();
}

VariableSecond &
NeighborMooseVariableInterface::neighborSecondOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOldNeighbor();
}

VariableSecond &
NeighborMooseVariableInterface::neighborSecondOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOlderNeighbor();
}

VariableTestSecond &
NeighborMooseVariableInterface::neighborSecondTest()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return const_cast<VariableTestSecond &>(_variable->secondPhiFaceNeighbor());
}

VariablePhiSecond &
NeighborMooseVariableInterface::neighborSecondPhi()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return const_cast<VariablePhiSecond &>(_mvi_assembly->secondPhiFaceNeighbor());
}
