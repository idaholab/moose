#include "VariableWarehouse.h"

VariableWarehouse::VariableWarehouse()
{
}

VariableWarehouse::~VariableWarehouse()
{
  for (std::vector<MooseVariable *>::iterator it = _vars.begin(); it != _vars.end(); ++it)
    delete *it;
}

void
VariableWarehouse::add(const std::string & var_name, MooseVariable *var)
{
  _var_name[var_name] = var;
  _vars.push_back(var);
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

std::vector<MooseVariable *> &
VariableWarehouse::all()
{
  return _vars;
}

std::set<MooseVariable *> &
VariableWarehouse::boundaryVars(unsigned int bnd)
{
  return _boundary_vars[bnd];
}
