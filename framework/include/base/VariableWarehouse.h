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
#include "MooseVariableScalar.h"


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
  void add(const std::string & var_name, MooseVariable *var);

  /**
   * Add a scalar variable
   * @param var_num The number of the variable
   * @param var Scalar variable
   */
  void add(const std::string & var_name, MooseVariableScalar * var);

  /**
   * Add a boundary variable
   * @param bnd The boundary id where this variable is defined
   * @param var The variable
   */
  void addBoundaryVar(unsigned int bnd, MooseVariable *var);
  /// Convenience function for adding coupled vars at once
  void addBoundaryVars(unsigned int bnd, const std::map<std::string, std::vector<MooseVariable *> > & vars);

  /**
   * Get a variable from the warehouse
   * @param var_name The name of the variable to retrieve
   * @return The retrieved variable
   */
  MooseVariable * getVariable(const std::string & var_name);

  /**
   * Get a scalar variable from the warehouse
   * @param var_num The number of the scalar variable to retrieve
   * @return The retrieved variable
   */
  MooseVariableScalar * getScalarVariable(const std::string & var_name);

  /**
   * Get the list of all variables
   * @return The list of variables
   */
  std::vector<MooseVariable *> & all();
  /**
   * Get the list of variables that needs to be reinitialized on a given boundary
   * @param bnd The boundary ID
   * @return The list of variables
   */
  std::set<MooseVariable *> & boundaryVars(unsigned int bnd);

  /**
   * Get the list of scalar variables
   * @return The list of scalar variables
   */
  std::vector<MooseVariableScalar *> & scalars();

protected:
  /// list of all variables
  std::vector<MooseVariable *> _vars;
  /// Name to variable mapping
  std::map<std::string, MooseVariable *> _var_name;
  /// Map to variables that need to be evaluated on a boundary
  std::map<unsigned int, std::set<MooseVariable *> > _boundary_vars;

  /// variable number to variable mapping
  std::map<std::string, MooseVariableScalar *> _scalar_var_map;
  /// list of all variables
  std::vector<MooseVariableScalar *> _scalar_vars;
};

#endif // VARIABLEWAREHOUSE_H
