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
#include "InitialCondition.h"
#include "ScalarInitialCondition.h"
#include "MooseTypes.h"

VariableWarehouse::VariableWarehouse()
{
}

VariableWarehouse::~VariableWarehouse()
{
  for (std::vector<MooseVariableBase *>::iterator it = _all.begin(); it != _all.end(); ++it)
    delete *it;

  for (std::map<std::string, std::map<SubdomainID, InitialCondition *> >::iterator it = _ics.begin(); it != _ics.end(); ++it)
    for (std::map<SubdomainID, InitialCondition *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      delete jt->second;
  for (std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.begin(); it != _scalar_ics.end(); ++it)
    delete it->second;
}

void
VariableWarehouse::add(const std::string & var_name, MooseVariableBase * var)
{
  _var_name[var_name] = var;
  _all.push_back(var);

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
VariableWarehouse::addBoundaryVar(BoundaryID bnd, MooseVariable *var)
{
  _boundary_vars[bnd].insert(var);
}

void
VariableWarehouse::addBoundaryVars(BoundaryID bnd, const std::map<std::string, std::vector<MooseVariable *> > & vars)
{
  for (std::map<std::string, std::vector<MooseVariable *> >::const_iterator it = vars.begin(); it != vars.end(); ++it)
    for (std::vector<MooseVariable *>::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      addBoundaryVar(bnd, *jt);
}

MooseVariableBase *
VariableWarehouse::getVariable(const std::string & var_name)
{
  return _var_name[var_name];
}

MooseVariableBase *
VariableWarehouse::getVariable(unsigned int var_number)
{
  if (var_number < _all.size())
    return _all[var_number];
  else
    return NULL;
}

const std::vector<MooseVariableBase *> &
VariableWarehouse::all()
{
  return _all;
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

void
VariableWarehouse::addInitialCondition(const std::string & var_name, SubdomainID blockid, InitialCondition * ic)
{
  std::string name;

  if (_ics[var_name].find(blockid) != _ics[var_name].end())                     // Two ics on the same block
    name = _ics[var_name][blockid]->name();
  else if (_ics[var_name].find(Moose::ANY_BLOCK_ID) != _ics[var_name].end())    // Two ics, first global
    name = _ics[var_name][Moose::ANY_BLOCK_ID]->name();
  else if (blockid == Moose::ANY_BLOCK_ID && _ics[var_name].size())             // Two ics, second global
    name = _ics[var_name].begin()->second->name();

  if (name != "")
    mooseError(std::string("Initial Conditions '") + name + "' and '" + ic->name()
               + "' are both defined on the same block.");

  _ics[var_name][blockid] = ic;
}

void
VariableWarehouse::addScalarInitialCondition(const std::string & var_name, ScalarInitialCondition * ic)
{
  _scalar_ics[var_name] = ic;
}

InitialCondition *
VariableWarehouse::getInitialCondition(const std::string & var_name, SubdomainID blockid)
{
  std::map<std::string, std::map<SubdomainID, InitialCondition *> >::iterator it = _ics.find(var_name);
  if (it != _ics.end())
  {
    std::map<SubdomainID, InitialCondition *>::iterator jt = it->second.find(blockid);
    if (jt != it->second.end())
      return jt->second;                        // we return the IC that was defined on the specified block (blockid)

    jt = it->second.find(Moose::ANY_BLOCK_ID);
    if (jt != it->second.end())
      return jt->second;                        // return the IC that lives everywhere
    else
      return NULL;                              // No IC there at all
  }
  else
    return NULL;
}

ScalarInitialCondition *
VariableWarehouse::getScalarInitialCondition(const std::string & var_name)
{
  std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.find(var_name);
  if (it != _scalar_ics.end())
    return it->second;
  else
    return NULL;
}

