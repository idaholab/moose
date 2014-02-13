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

#include "MooseTypes.h"

#include <vector>
#include <map>
#include <set>

class MooseVariableBase;
class MooseVariable;
class MooseVariableScalar;

/**
 * Holds variables and provides some services
 */
class VariableWarehouse
{
public:
  VariableWarehouse();
  virtual ~VariableWarehouse();

  /**
   * Add a variable
   * @param var_name The name of the variable
   * @param var Variable
   */
  void add(const std::string & var_name, MooseVariableBase * var);

  /**
   * Add a boundary variable
   * @param bnd The boundary id where this variable is defined
   * @param var The variable
   */
  void addBoundaryVar(BoundaryID bnd, MooseVariable *var);
  /// Convenience function for adding coupled vars at once
  void addBoundaryVars(BoundaryID bnd, const std::map<std::string, std::vector<MooseVariable *> > & vars);

  /**
   * Get a variable from the warehouse
   * @param var_name The name of the variable to retrieve
   * @return The retrieved variable
   */
  MooseVariableBase * getVariable(const std::string & var_name);

  /**
   * Get a variable from the warehouse
   * @param var_num The number of the variable to retrieve
   * @return The retrieved variable
   */
  MooseVariableBase * getVariable(unsigned int var_number);

  /**
   * Get the list of all variable names
   * @return The list of variable names
   */
  const std::vector<VariableName> & names() const;

  /**
   * Get the list of all variables
   * @return The list of variables
   */
  const std::vector<MooseVariableBase *> & all();

  /**
   * Get the list of variables
   * @return The list of variables
   */
  const std::vector<MooseVariable *> & variables();

  /**
   * Get the list of variables that needs to be reinitialized on a given boundary
   * @param bnd The boundary ID
   * @return The list of variables
   */
  const std::set<MooseVariable *> & boundaryVars(BoundaryID bnd);

  /**
   * Get the list of scalar variables
   * @return The list of scalar variables
   */
  const std::vector<MooseVariableScalar *> & scalars();

protected:
  /// list of variable names
  std::vector<VariableName> _names;
  /// list of all variables
  std::vector<MooseVariableBase *> _all;
  /// list of "normal" variables
  std::vector<MooseVariable *> _vars;
  /// Name to variable mapping
  std::map<std::string, MooseVariableBase *> _var_name;
  /// Map to variables that need to be evaluated on a boundary
  std::map<BoundaryID, std::set<MooseVariable *> > _boundary_vars;

  /// list of all variables
  std::vector<MooseVariableScalar *> _scalar_vars;
};

#endif // VARIABLEWAREHOUSE_H
