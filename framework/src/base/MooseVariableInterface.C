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

#include "MooseVariableInterface.h"
#include "Moose.h"
#include "Problem.h"
#include "SubProblem.h"

MooseVariableInterface::MooseVariableInterface(InputParameters & parameters, bool nodal) :
    _nodal(nodal)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  _variable = &problem.getVariable(tid, parameters.get<std::string>("variable"));
  _mvi_assembly = &problem.assembly(tid);
}

MooseVariableInterface::~MooseVariableInterface()
{
}


VariableValue &
MooseVariableInterface::value()
{
  if (_nodal)
    return _variable->nodalSln();
  else
    return _variable->sln();
}

VariableValue &
MooseVariableInterface::valueOld()
{
  if (_nodal)
    return _variable->nodalSlnOld();
  else
    return _variable->slnOld();
}

VariableValue &
MooseVariableInterface::valueOlder()
{
  if (_nodal)
    return _variable->nodalSlnOlder();
  else
    return _variable->slnOlder();
}

VariableValue &
MooseVariableInterface::dot()
{
  if (_variable->kind() == Moose::VAR_AUXILIARY)
    mooseError("Can't get time derivative of auxiliary _variables!");

  if (_nodal)
    return _variable->nodalSlnDot();
  else
    return _variable->uDot();
}

VariableValue &
MooseVariableInterface::dotDu()
{
  if (_variable->kind() == Moose::VAR_AUXILIARY)
    mooseError("Can't get time derivative of auxiliary _variables!");

  if (_nodal)
    return _variable->nodalSlnDuDotDu();
  else
    return _variable->duDotDu();
}


VariableGradient &
MooseVariableInterface::gradient()
{
  if (_nodal)
    mooseError("Nodal _variables do not have gradients");

  return _variable->gradSln();
}

VariableGradient &
MooseVariableInterface::gradientOld()
{
  if (_nodal)
    mooseError("Nodal _variables do not have gradients");

  return _variable->gradSlnOld();
}

VariableGradient &
MooseVariableInterface::gradientOlder()
{
  if (_nodal)
    mooseError("Nodal _variables do not have gradients");

  return _variable->gradSlnOlder();
}

VariableSecond &
MooseVariableInterface::second()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return _variable->secondSln();
}

VariableSecond &
MooseVariableInterface::secondOld()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return _variable->secondSlnOld();
}

VariableSecond &
MooseVariableInterface::secondOlder()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return _variable->secondSlnOlder();
}

VariableTestSecond &
MooseVariableInterface::secondTest()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return const_cast<std::vector<std::vector<RealTensor> > &>(_variable->secondPhi());
}

VariablePhiSecond &
MooseVariableInterface::secondPhi()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return const_cast<VariablePhiSecond &>(_mvi_assembly->secondPhi());
}

// Neighbor values

NeighborMooseVariableInterface::NeighborMooseVariableInterface(InputParameters & parameters, bool nodal) :
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
    mooseError("Nodal _variables do not have gradients");

  return _variable->gradSlnNeighbor();
}

VariableGradient &
NeighborMooseVariableInterface::neighborGradientOld()
{
  if (_nodal)
    mooseError("Nodal _variables do not have gradients");

  return _variable->gradSlnOldNeighbor();
}

VariableGradient &
NeighborMooseVariableInterface::neighborGradientOlder()
{
  if (_nodal)
    mooseError("Nodal _variables do not have gradients");

  return _variable->gradSlnOlderNeighbor();
}

VariableSecond &
NeighborMooseVariableInterface::neighborSecond()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return _variable->secondSlnNeighbor();
}

VariableSecond &
NeighborMooseVariableInterface::neighborSecondOld()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return _variable->secondSlnOldNeighbor();
}

VariableSecond &
NeighborMooseVariableInterface::neighborSecondOlder()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return _variable->secondSlnOlderNeighbor();
}

VariableTestSecond &
NeighborMooseVariableInterface::neighborSecondTest()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return const_cast<std::vector<std::vector<RealTensor> > &>(_variable->secondPhiFaceNeighbor());
}

VariablePhiSecond &
NeighborMooseVariableInterface::neighborSecondPhi()
{
  if (_nodal)
    mooseError("Nodal _variables do not have second derivatives");

  return const_cast<VariablePhiSecond &>(_mvi_assembly->secondPhiFaceNeighbor());
}
