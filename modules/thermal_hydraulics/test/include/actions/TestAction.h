//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Base class for adding common actions for testing
 */
class TestAction : public Action
{
public:
  TestAction(const InputParameters & params);

  virtual void act();

protected:
  /**
   * Adds the mesh with a provided number of elements in x direction
   *
   * @param[in] nx  number of elements in x direction
   */
  void addMeshInternal(const unsigned int & nx);

  /**
   * Adds all non-mesh objects
   */
  virtual void addObjects();

  /**
   * Adds scalar variables
   *
   * @param[in] names  names of the variables to add
   * @param[in] values  values of the variables to add
   */
  void addScalarVariables(const std::vector<VariableName> & names,
                          const std::vector<FunctionName> & values);

  /**
   * Adds aux variables
   */
  virtual void addAuxVariables();

  /**
   * Adds materials
   */
  virtual void addMaterials();

  /**
   * Adds a solution variable
   *
   * @param[in] var_name  name of the variable to add
   * @param[in] family  variable family
   * @param[in] order  variable order
   * @param[in] scaling  scaling factor to apply to variable
   */
  void addSolutionVariable(const VariableName & var_name,
                           const std::string & family = "LAGRANGE",
                           const std::string & order = "FIRST",
                           const Real & scaling = 1.0);

  /**
   * Adds an aux variable
   *
   * @param[in] var_name  name of the variable to add
   * @param[in] fe_family  finite element family
   * @param[in] fe_order   finite element order
   */
  void addAuxVariable(const VariableName & var_name,
                      const std::string & fe_family,
                      const std::string & fe_order);

  /**
   * Adds a constant initial condition
   *
   * @param[in] var_name  name of the variable for which to add initial condition
   * @param[in] value  value of the initial condition
   */
  void addConstantIC(const VariableName & var_name, const Real & value);

  /**
   * Adds a function initial condition
   *
   * @param[in] var_name  name of the variable for which to add initial condition
   * @param[in] function_name  names of the IC function
   */
  void addFunctionIC(const VariableName & var_name, const FunctionName & function_name);

  /**
   * Adds the mesh
   */
  virtual void addMesh();

  /**
   * Adds the preconditioner
   */
  virtual void addPreconditioner();

  /**
   * Adds the executioner
   */
  virtual void addExecutioner();

  /**
   * Add output
   */
  virtual void addOutput();

  /**
   * Adds the initial conditions
   */
  virtual void addInitialConditions() = 0;

  /**
   * Adds the solution variables
   */
  virtual void addSolutionVariables() = 0;

  /**
   * Adds user objects
   */
  virtual void addUserObjects() = 0;

  /// Default for option to use a transient executioner
  bool _default_use_transient_executioner;

  /// List of scalar variables to add
  const std::vector<VariableName> _scalar_variables;
  /// List of values for the scalar variables to add
  const std::vector<FunctionName> _scalar_variable_values;

  /// List of aux variables to add
  const std::vector<VariableName> _aux_variables;
  /// List of function names for aux variables to add
  const std::vector<FunctionName> _aux_variable_values;

  /// List of material properties to add
  const std::vector<std::string> _mat_property_names;
  /// List of function names for material properties to add
  const std::vector<FunctionName> _mat_property_values;

  /// Default FE family
  const std::string _fe_family;
  /// Default FE order
  const std::string _fe_order;
  /// True for setting up testing with AD, false otherwise
  const bool & _ad;

public:
  static InputParameters validParams();
};
