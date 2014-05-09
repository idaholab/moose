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

#include "NeighborCoupleable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "FEProblem.h"


NeighborCoupleable::NeighborCoupleable(InputParameters & parameters, bool nodal) :
    Coupleable(parameters, nodal)
{
}

NeighborCoupleable::~NeighborCoupleable()
{
}

VariableValue &
NeighborCoupleable::coupledNeighborValue(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
    return (_c_is_implicit) ? var->nodalSlnNeighbor() : var->nodalSlnOldNeighbor();
  else
    return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOld(const std::string & var_name, unsigned int comp)
{
  validateExecutionerType(var_name);

  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
    return (_c_is_implicit) ? var->nodalSlnOldNeighbor() : var->nodalSlnOlderNeighbor();
  else
    return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
}

VariableValue &
NeighborCoupleable::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp)
{
  validateExecutionerType(var_name);

  MooseVariable * var = getVar(var_name, comp);
  if (_nodal)
  {
    if (_c_is_implicit)
      return var->nodalSlnOlderNeighbor();
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

VariableGradient &
NeighborCoupleable::coupledNeighborGradient(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name);
  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

VariableGradient &
NeighborCoupleable::coupledNeighborGradientOlder(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have gradients");

  validateExecutionerType(var_name);
  MooseVariable * var = getVar(var_name, comp);
  if (_c_is_implicit)
    return var->gradSlnOlderNeighbor();
  else
    mooseError("Older values not available for explicit schemes");
}

VariableSecond &
NeighborCoupleable::coupledNeighborSecond(const std::string & var_name, unsigned int comp)
{
  if (_nodal)
    mooseError("Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOldNeighbor();
}
