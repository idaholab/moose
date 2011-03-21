#ifndef VARIABLEWAREHOUSE_H_
#define VARIABLEWAREHOUSE_H_

#include <vector>
#include <map>
#include <set>

#include "Variable.h"


namespace Moose {

/**
 * Holds variables and provides some services
 */
class VariableWarehouse
{
public:
  VariableWarehouse();
  virtual ~VariableWarehouse();

  void add(const std::string & var_name, Variable *var);
  void addBoundaryVar(unsigned int bnd, Variable *var);

  Variable *getVariable(const std::string & var_name);

  std::vector<Variable *> &all();
  std::set<Variable *> &boundaryVars(unsigned int bnd);

protected:
  std::vector<Variable *> _vars;                                        /// list of all variables
  std::map<std::string, Variable *> _var_name;                          ///
  std::map<unsigned int, std::set<Variable *> > _boundary_vars;         /// Map to variables that need to be evaluated on a boundary
};

} // namespace

#endif // KERNELWAREHOUSE_H_
