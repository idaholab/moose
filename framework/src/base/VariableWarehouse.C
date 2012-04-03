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

VariableWarehouse::VariableWarehouse()
{
}

VariableWarehouse::~VariableWarehouse()
{
  for (std::vector<MooseVariable *>::iterator it = _vars.begin(); it != _vars.end(); ++it)
    delete *it;
  for (std::vector<MooseVariableScalar *>::iterator it = _scalar_vars.begin(); it != _scalar_vars.end(); ++it)
    delete *it;

  for (std::map<std::string, std::map<subdomain_id_type, InitialCondition *> >::iterator it = _ics.begin(); it != _ics.end(); ++it)
    for (std::map<subdomain_id_type, InitialCondition *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      delete jt->second;
  for (std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.begin(); it != _scalar_ics.end(); ++it)
    delete it->second;
}

void
VariableWarehouse::add(const std::string & var_name, MooseVariable *var)
{
  _var_name[var_name] = var;
  _vars.push_back(var);
}

void
VariableWarehouse::add(const std::string & var_name, MooseVariableScalar * var)
{
  _scalar_var_map[var_name] = var;
  _scalar_vars.push_back(var);
}


void
VariableWarehouse::addBoundaryVar(unsigned int bnd, MooseVariable *var)
{
  _boundary_vars[bnd].insert(var);
}

void
VariableWarehouse::addBoundaryVars(unsigned int bnd, const std::map<std::string, std::vector<MooseVariable *> > & vars)
{
  for (std::map<std::string, std::vector<MooseVariable *> >::const_iterator it = vars.begin(); it != vars.end(); ++it)
    for (std::vector<MooseVariable *>::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      addBoundaryVar(bnd, *jt);
}

MooseVariable *
VariableWarehouse::getVariable(const std::string & var_name)
{
  return _var_name[var_name];
}

MooseVariableScalar *
VariableWarehouse::getScalarVariable(const std::string & var_name)
{
  return _scalar_var_map[var_name];
}

std::vector<MooseVariable *> &
VariableWarehouse::all()
{
  return _vars;
}

std::vector<MooseVariableScalar *> &
VariableWarehouse::scalars()
{
  return _scalar_vars;
}

std::set<MooseVariable *> &
VariableWarehouse::boundaryVars(unsigned int bnd)
{
  return _boundary_vars[bnd];
}

void
VariableWarehouse::addInitialCondition(const std::string & var_name, subdomain_id_type blockid, InitialCondition * ic)
{
  _ics[var_name][blockid] = ic;
}

void
VariableWarehouse::addScalarInitialCondition(const std::string & var_name, ScalarInitialCondition * ic)
{
  _scalar_ics[var_name] = ic;
}

InitialCondition *
VariableWarehouse::getInitialCondition(const std::string & var_name, subdomain_id_type blockid)
{
  std::map<std::string, std::map<subdomain_id_type, InitialCondition *> >::iterator it = _ics.find(var_name);
  if (it != _ics.end())
  {
    std::map<subdomain_id_type, InitialCondition *>::iterator jt = it->second.find(blockid);
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

