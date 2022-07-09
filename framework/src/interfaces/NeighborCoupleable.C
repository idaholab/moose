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
                                       bool neighbor_nodal,
                                       bool is_fv)
  : Coupleable(moose_object, nodal, is_fv), _neighbor_nodal(neighbor_nodal)
{
}

const VariableValue &
NeighborCoupleable::coupledNeighborValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);
  if (_neighbor_nodal)
    return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
  else
    return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

std::vector<const VariableValue *>
NeighborCoupleable::coupledNeighborValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp)
  { return &coupledNeighborValue(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

const ADVariableValue &
NeighborCoupleable::adCoupledNeighborValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return *getADDefaultValue(var_name);

  if (_neighbor_nodal)
    mooseError("adCoupledNeighborValue cannot be used for nodal compute objects at this time");
  if (!_c_is_implicit)
    mooseError("adCoupledNeighborValue returns a data structure with derivatives. Explicit schemes "
               "use old solution data which do not have derivatives so adCoupledNeighborValue is "
               "not appropriate. Please use coupledNeighborValue instead");

  return var->adSlnNeighbor();
}

const ADVariableValue &
NeighborCoupleable::adCoupledNeighborValueDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!_c_is_implicit)
    mooseError(
        "adCoupledNeighborValueDot returns a data structure with derivatives. Explicit schemes "
        "use old solution data which do not have derivatives so adCoupledNeighborValueDot is "
        "not appropriate. Please use coupledNeighborValueDot instead");

  if (_neighbor_nodal)
    mooseError("adCoupledNeighborValueDot cannot be used for nodal compute objects at this time");
  else
    return var->adUDotNeighbor();
}

std::vector<const ADVariableValue *>
NeighborCoupleable::adCoupledNeighborValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp)
  { return &adCoupledNeighborValue(var_name, comp); };
  return coupledVectorHelper<const ADVariableValue *>(var_name, func);
}

const ADVectorVariableValue &
NeighborCoupleable::adCoupledVectorNeighborValue(const std::string & var_name,
                                                 unsigned int comp) const
{
  auto var = getVarHelper<MooseVariableField<RealVectorValue>>(var_name, comp);

  if (!var)
    return *getADDefaultVectorValue(var_name);

  if (_neighbor_nodal)
    mooseError(
        "adCoupledVectorNeighborValue cannot be used for nodal compute objects at this time");
  if (!_c_is_implicit)
    mooseError(
        "adCoupledVectorNeighborValue returns a data structure with derivatives. Explicit schemes "
        "use old solution data which do not have derivatives so adCoupledVectorNeighborValue is "
        "not appropriate. Please use coupledNeighborValue instead");

  return var->adSlnNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return var->dofValuesDotNeighbor();
  else
    return var->uDotNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueDotDu(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return var->dofValuesDuDotDuNeighbor();
  else
    return var->duDotDuNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueOld(const std::string & var_name, unsigned int comp) const
{
  validateExecutionerType(var_name, "coupledNeighborValueOld");

  const auto * var = getVar(var_name, comp);
  if (_neighbor_nodal)
    return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
  else
    return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp) const
{
  validateExecutionerType(var_name, "coupledNeighborValueOlder");

  const auto * var = getVar(var_name, comp);
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
NeighborCoupleable::coupledNeighborGradient(const std::string & var_name, unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");

  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

std::vector<const VariableGradient *>
NeighborCoupleable::coupledNeighborGradients(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp)
  { return &coupledNeighborGradient(var_name, comp); };
  return coupledVectorHelper<const VariableGradient *>(var_name, func);
}

const VariableGradient &
NeighborCoupleable::coupledNeighborGradientOld(const std::string & var_name,
                                               unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name, "coupledNeighborGradientOld");
  const auto * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VariableGradient &
NeighborCoupleable::coupledNeighborGradientOlder(const std::string & var_name,
                                                 unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name, "coupledNeighborGradientOlder");
  const auto * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

const ADVariableGradient &
NeighborCoupleable::adCoupledNeighborGradient(const std::string & var_name, unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have gradients");
  if (!_c_is_implicit)
    mooseError(
        "adCoupledNeighborGradient returns a data structure with derivatives. Explicit schemes "
        "use old solution data which do not have derivatives so adCoupledNeighborGradient is "
        "not appropriate. Please use coupledNeighborGradient instead");

  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);
  return var->adGradSlnNeighbor();
}

const VectorVariableGradient &
NeighborCoupleable::coupledVectorNeighborGradient(const std::string & var_name,
                                                  unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  const auto * var = getVectorVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VectorVariableGradient &
NeighborCoupleable::coupledVectorNeighborGradientOld(const std::string & var_name,
                                                     unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledVectorNeighborGradientOld");
  const auto * var = getVectorVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VectorVariableGradient &
NeighborCoupleable::coupledVectorNeighborGradientOlder(const std::string & var_name,
                                                       unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledVectorNeighborGradientOlder");
  const auto * var = getVectorVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

const ArrayVariableValue &
NeighborCoupleable::coupledArrayNeighborValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
  if (_neighbor_nodal)
    return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
  else
    return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

const ArrayVariableGradient &
NeighborCoupleable::coupledArrayNeighborGradient(const std::string & var_name,
                                                 unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  const auto * var = getArrayVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const ArrayVariableGradient &
NeighborCoupleable::coupledArrayNeighborGradientOld(const std::string & var_name,
                                                    unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledArrayNeighborGradientOld");
  const auto * var = getArrayVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const ArrayVariableGradient &
NeighborCoupleable::coupledArrayNeighborGradientOlder(const std::string & var_name,
                                                      unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledArrayNeighborGradientOlder");
  const auto * var = getArrayVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableSecond &
NeighborCoupleable::coupledNeighborSecond(const std::string & var_name, unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("Nodal variables do not have second derivatives");

  const auto * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOldNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborDofValues(const std::string & var_name, unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("nodal objects should not call coupledDofValues");

  const auto * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborDofValuesOld(const std::string & var_name,
                                                unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("nodal objects should not call coupledDofValuesOld");

  validateExecutionerType(var_name, "coupledNeighborDofValuesOld");
  const auto * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
}

const VariableValue &
NeighborCoupleable::coupledNeighborDofValuesOlder(const std::string & var_name,
                                                  unsigned int comp) const
{
  if (_neighbor_nodal)
    mooseError("nodal objects should not call coupledDofValuesOlder");

  validateExecutionerType(var_name, "coupledNeighborDofValuesOlder");
  const auto * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->dofValuesOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}
