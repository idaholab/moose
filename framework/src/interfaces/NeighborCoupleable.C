//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NeighborCoupleable.h"

#include "FEProblem.h"
#include "MooseError.h" // mooseDeprecated
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"

NeighborCoupleable::NeighborCoupleable(const MooseObject * moose_object,
                                       bool nodal,
                                       bool neighbor_nodal)
  : Coupleable(moose_object, nodal), _neighbor_nodal(neighbor_nodal)
{
}

const VariableValue &
NeighborCoupleable::coupledNeighborValue(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
  else
    return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return var->dofValuesDotNeighbor();
  else
    return var->uDotNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueDotResidual(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return var->dofValuesDotNeighborResidual();
  else
    return var->uDotNeighborResidual();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return var->dofValuesDuDotDuNeighbor();
  else
    return var->duDotDuNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueOld(const std::string & var_name, unsigned int comp)
{
  validateExecutionerType(var_name, "coupledNeighborValueOld");

  MooseVariable * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
  else
    return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp)
{
  validateExecutionerType(var_name, "coupledNeighborValueOlder");

  MooseVariable * var = getVar(var_name, comp);
  if (_neighbor_nodal)
  {
    if (_c_is_implicit)
      return var->dofValuesOlderNeighbor();
    else
      mooseError("Older values not available for explicit schemes");
  }
  else
  {
    if (_c_is_implicit)
      return var->slnOlderNeighbor();
    else
      mooseError("Older values not available for explicit schemes");
  }
}

const VariableGradient &
NeighborCoupleable::coupledNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VariableGradient &
NeighborCoupleable::coupledNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name, "coupledNeighborGradientOld");
  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VariableGradient &
NeighborCoupleable::coupledNeighborGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name, "coupledNeighborGradientOlder");
  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

const VectorVariableGradient &
NeighborCoupleable::coupledVectorNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  VectorMooseVariable * var = getVectorVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VectorVariableGradient &
NeighborCoupleable::coupledVectorNeighborGradientOld(const std::string & var_name,
                                                     unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledVectorNeighborGradientOld");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VectorVariableGradient &
NeighborCoupleable::coupledVectorNeighborGradientOlder(const std::string & var_name,
                                                       unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledVectorNeighborGradientOlder");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

const ArrayVariableValue &
NeighborCoupleable::coupledArrayNeighborValue(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (_neighbor_nodal)
    return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
  else
    return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

const ArrayVariableGradient &
NeighborCoupleable::coupledArrayNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const ArrayVariableGradient &
NeighborCoupleable::coupledArrayNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledArrayNeighborGradientOld");
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const ArrayVariableGradient &
NeighborCoupleable::coupledArrayNeighborGradientOlder(const std::string & var_name,
                                                      unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledArrayNeighborGradientOlder");
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableSecond &
NeighborCoupleable::coupledNeighborSecond(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOldNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborDofValues(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("nodal objects should not call coupledDofValues");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborDofValuesOld(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("nodal objects should not call coupledDofValuesOld");

  validateExecutionerType(var_name, "coupledNeighborDofValuesOld");
  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborDofValuesOlder(const std::string & var_name, unsigned int comp)
{
  if (_neighbor_nodal)
    mooseError("nodal objects should not call coupledDofValuesOlder");

  validateExecutionerType(var_name, "coupledNeighborDofValuesOlder");
  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->dofValuesOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}
