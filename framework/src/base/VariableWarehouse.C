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

#include "VariableWarehouse.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "MooseTypes.h"

VariableWarehouse::VariableWarehouse() {}

VariableWarehouse::~VariableWarehouse()
{
  for (auto & var : _all_objects)
    delete var;
}

void
VariableWarehouse::add(const std::string & var_name, MooseVariableBase * var)
{
  _names.push_back(var_name);
  _var_name[var_name] = var;
  _all_objects.push_back(var);

  if (dynamic_cast<MooseVariable *>(var) != NULL)
  {
    _vars.push_back(dynamic_cast<MooseVariable *>(var));
  }
  else if (dynamic_cast<MooseVariableScalar *>(var) != NULL)
  {
    _scalar_vars.push_back(dynamic_cast<MooseVariableScalar *>(var));
  }
  else
    mooseError("Unknown variable class passed into VariableWarehouse. Attempt to hack us?");
}

void
VariableWarehouse::addBoundaryVar(BoundaryID bnd, MooseVariable * var)
{
  _boundary_vars[bnd].insert(var);
}

void
VariableWarehouse::addBoundaryVar(const std::set<BoundaryID> & boundary_ids, MooseVariable * var)
{
  for (const auto & bid : boundary_ids)
    addBoundaryVar(bid, var);
}

void
VariableWarehouse::addBoundaryVars(const std::set<BoundaryID> & boundary_ids,
                                   const std::map<std::string, std::vector<MooseVariable *>> & vars)
{
  for (const auto & bid : boundary_ids)
    for (const auto & it : vars)
      for (const auto & var : it.second)
        addBoundaryVar(bid, var);
}

MooseVariableBase *
VariableWarehouse::getVariable(const std::string & var_name)
{
  return _var_name[var_name];
}

MooseVariableBase *
VariableWarehouse::getVariable(unsigned int var_number)
{
  if (var_number < _all_objects.size())
    return _all_objects[var_number];
  else
    return NULL;
}

const std::vector<VariableName> &
VariableWarehouse::names() const
{
  return _names;
}

const std::vector<MooseVariable *> &
VariableWarehouse::variables()
{
  return _vars;
}

const std::vector<MooseVariableScalar *> &
VariableWarehouse::scalars()
{
  return _scalar_vars;
}

const std::set<MooseVariable *> &
VariableWarehouse::boundaryVars(BoundaryID bnd)
{
  return _boundary_vars[bnd];
}
