#include "VariableWarehouse.h"

namespace Moose {

VariableWarehouse::VariableWarehouse()
{
}

VariableWarehouse::~VariableWarehouse()
{
}

void
VariableWarehouse::add(const std::string & var_name, Variable *var)
{
  _var_name[var_name] = var;
  _vars.push_back(var);
}

void
VariableWarehouse::addBoundaryVar(unsigned int bnd, Variable *var)
{
  _boundary_vars[bnd].insert(var);
}

void
VariableWarehouse::addBoundaryVars(unsigned int bnd, const std::map<std::string, std::vector<Variable *> > & vars)
{
  for (std::map<std::string, std::vector<Variable *> >::const_iterator it = vars.begin(); it != vars.end(); ++it)
    for (std::vector<Variable *>::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      addBoundaryVar(bnd, *jt);
}

Variable *
VariableWarehouse::getVariable(const std::string & var_name)
{
  return _var_name[var_name];
}

std::vector<Variable *> &
VariableWarehouse::all()
{
  return _vars;
}

std::set<Variable *> &
VariableWarehouse::boundaryVars(unsigned int bnd)
{
  return _boundary_vars[bnd];
}


} // namespace
