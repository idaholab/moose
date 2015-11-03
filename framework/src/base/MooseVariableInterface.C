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
#include "Problem.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

MooseVariableInterface::MooseVariableInterface(const InputParameters & parameters, bool nodal, std::string var_param_name) :
    _nodal(nodal)
{
  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Try the scalar version first
  std::string variable_name = parameters.getMooseType(var_param_name);
  if (variable_name == "")
    // When using vector variables, we are only going to use the first one in the list at the interface level...
    variable_name = parameters.getVecMooseType(var_param_name)[0];

  _variable = &problem.getVariable(tid, variable_name);

  _mvi_assembly = &problem.assembly(tid);
}

MooseVariableInterface::~MooseVariableInterface()
{
}

MooseVariable *
MooseVariableInterface::mooseVariable()
{
  return _variable;
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
  if (_nodal)
    return _variable->nodalSlnDot();
  else
    return _variable->uDot();
}

VariableValue &
MooseVariableInterface::dotDu()
{
  if (_nodal)
    return _variable->nodalSlnDuDotDu();
  else
    return _variable->duDotDu();
}


VariableGradient &
MooseVariableInterface::gradient()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSln();
}

VariableGradient &
MooseVariableInterface::gradientOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOld();
}

VariableGradient &
MooseVariableInterface::gradientOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  return _variable->gradSlnOlder();
}

VariableSecond &
MooseVariableInterface::second()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSln();
}

VariableSecond &
MooseVariableInterface::secondOld()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOld();
}

VariableSecond &
MooseVariableInterface::secondOlder()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return _variable->secondSlnOlder();
}

VariableTestSecond &
MooseVariableInterface::secondTest()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return const_cast<VariableTestSecond &>(_variable->secondPhi());
}

VariablePhiSecond &
MooseVariableInterface::secondPhi()
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return const_cast<VariablePhiSecond &>(_mvi_assembly->secondPhi());
}
