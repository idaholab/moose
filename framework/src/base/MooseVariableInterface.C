//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableInterface.h"

#include "Assembly.h"
#include "MooseError.h" // mooseDeprecated
#include "MooseTypes.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"

MooseVariableInterface::MooseVariableInterface(const MooseObject * moose_object,
                                               bool nodal,
                                               std::string var_param_name)
  : _nodal(nodal)
{
  const InputParameters & parameters = moose_object->parameters();

  SubProblem & problem = *parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Try the scalar version first
  std::string variable_name = parameters.getMooseType(var_param_name);
  if (variable_name == "")
    // When using vector variables, we are only going to use the first one in the list at the
    // interface level...
    variable_name = parameters.getVecMooseType(var_param_name)[0];

  _variable = &problem.getVariable(tid, variable_name);

  _mvi_assembly = &problem.assembly(tid);
}

MooseVariableInterface::~MooseVariableInterface() {}

MooseVariable *
MooseVariableInterface::mooseVariable()
{
  return _variable;
}

const VariableValue &
MooseVariableInterface::value()
{
  if (_nodal)
    return _variable->nodalSln();
  else
    return _variable->sln();
}

const VariableValue &
MooseVariableInterface::valueOld()
{
  if (_nodal)
    return _variable->nodalSlnOld();
  else
    return _variable->slnOld();
}

const VariableValue &
MooseVariableInterface::valueOlder()
{
  if (_nodal)
    return _variable->nodalSlnOlder();
  else
    return _variable->slnOlder();
}

const VariableValue &
MooseVariableInterface::dot()
{
  if (_nodal)
    return _variable->nodalSlnDot();
  else
    return _variable->uDot();
}

const VariableValue &
MooseVariableInterface::dotDu()
{
  if (_nodal)
    return _variable->nodalSlnDuDotDu();
  else
    return _variable->duDotDu();
}

const VariableGradient &
MooseVariableInterface::gradient()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSln();
}

const VariableGradient &
MooseVariableInterface::gradientOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOld();
}

const VariableGradient &
MooseVariableInterface::gradientOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOlder();
}

const VariableSecond &
MooseVariableInterface::second()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSln();
}

const VariableSecond &
MooseVariableInterface::secondOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOld();
}

const VariableSecond &
MooseVariableInterface::secondOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOlder();
}

const VariableTestSecond &
MooseVariableInterface::secondTest()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondPhi();
}

const VariableTestSecond &
MooseVariableInterface::secondTestFace()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondPhiFace();
}

const VariablePhiSecond &
MooseVariableInterface::secondPhi()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _mvi_assembly->secondPhi();
}

const VariablePhiSecond &
MooseVariableInterface::secondPhiFace()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _mvi_assembly->secondPhiFace();
}
