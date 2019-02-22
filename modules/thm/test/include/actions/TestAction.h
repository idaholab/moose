#ifndef TESTACTION_H
#define TESTACTION_H

#include "Action.h"

class TestAction;

template <>
InputParameters validParams<TestAction>();

/**
 * Base class for adding common actions for testing
 */
class TestAction : public Action
{
public:
  TestAction(InputParameters params);

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
                          const std::vector<Real> & values);

  /**
   * Adds constant aux variables
   *
   * @param[in] fe_family  finite element family
   * @param[in] fe_order   finite element order
   */
  void addConstantAuxVariables(const std::string & fe_family, const std::string & fe_order);

  /**
   * Adds constant aux variables
   *
   * @param[in] names  names of the variables to add
   * @param[in] values  values of the variables to add
   * @param[in] fe_family  finite element family
   * @param[in] fe_order   finite element order
   */
  void addConstantAuxVariables(const std::vector<VariableName> & names,
                               const std::vector<Real> & values,
                               const std::string & fe_family,
                               const std::string & fe_order);

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
   * Adds constant solution variables
   *
   * @param[in] names  names of the variables to add
   * @param[in] values  values of the variables to add
   * @param[in] family  variable family
   * @param[in] order  variable order
   */
  void addConstantSolutionVariables(const std::vector<VariableName> & names,
                                    const std::vector<Real> & values,
                                    const std::string & family = "LAGRANGE",
                                    const std::string & order = "FIRST");

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
   * Adds the initial conditions
   */
  virtual void addInitialConditions() = 0;

  /**
   * Adds the solution variables
   */
  virtual void addSolutionVariables() = 0;

  /**
   * Adds the non-constant aux variables and their initial conditions
   */
  virtual void addNonConstantAuxVariables() = 0;

  /**
   * Adds materials
   */
  virtual void addMaterials() = 0;

  /**
   * Adds user objects
   */
  virtual void addUserObjects() = 0;

  /// Default for option to use a transient executioner
  bool _default_use_transient_executioner;

  /// List of scalar variables to add
  const std::vector<VariableName> _scalar_variables;
  /// List of values for the scalar variables to add
  const std::vector<Real> _scalar_variable_values;

  /// List of constant aux variables to add
  const std::vector<VariableName> _constant_aux_variables;
  /// List of values for the constant aux variables to add
  const std::vector<Real> _constant_aux_variable_values;

  /// Default FE family
  const std::string _fe_family;
  /// Default FE order
  const std::string _fe_order;
};

#endif /* TESTACTION_H */
