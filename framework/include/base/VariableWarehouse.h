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

#ifndef VARIABLEWAREHOUSE_H
#define VARIABLEWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "MooseVariable.h"


/**
 * Holds variables and provides some services
 */
class VariableWarehouse
{
public:
  VariableWarehouse();
  virtual ~VariableWarehouse();

  void add(const std::string & var_name, MooseVariable *var);
  void addBoundaryVar(unsigned int bnd, MooseVariable *var);
  /// Convenience function for adding coupled vars at once
  void addBoundaryVars(unsigned int bnd, const std::map<std::string, std::vector<MooseVariable *> > & vars);

  MooseVariable *getVariable(const std::string & var_name);

  std::vector<MooseVariable *> &all();
  std::set<MooseVariable *> &boundaryVars(unsigned int bnd);

protected:
  std::vector<MooseVariable *> _vars;                                        /// list of all variables
  std::map<std::string, MooseVariable *> _var_name;                          ///
  std::map<unsigned int, std::set<MooseVariable *> > _boundary_vars;         /// Map to variables that need to be evaluated on a boundary
};

#endif // KERNELWAREHOUSE_H
