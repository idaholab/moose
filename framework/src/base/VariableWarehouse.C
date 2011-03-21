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
