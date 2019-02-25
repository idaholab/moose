#ifndef FLOWMODELSETUP_H
#define FLOWMODELSETUP_H

#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "InputParameters.h"
#include "MooseApp.h"
#include "MooseTypes.h"

class FlowModelSetup;

template <>
InputParameters validParams<FlowModelSetup>();

/**
 * Base helper class to provide interfaces to common flow model setup functions
 */
class FlowModelSetup
{
public:
  FlowModelSetup(InputParameters params);

protected:
  virtual void addInitialConditions() = 0;
  virtual void addSolutionVariables() = 0;
  virtual void addNonConstantAuxVariables() = 0;
  virtual void addMaterials();
  virtual void addUserObjects() = 0;

  /**
   * Adds a solution variable
   *
   * @param[in] var_name  name of the variable to add
   * @param[in] scaling  scaling factor to apply to variable
   */
  void addSolutionVariable(const VariableName & var_name, const Real & scaling = 1.0);

  /**
   * Adds an aux variable
   *
   * @param[in] var_name  name of the variable to add
   */
  void addAuxVariable(const VariableName & var_name);

  /**
   * Adds a function initial condition
   *
   * @param[in] var_name  name of the variable for which to add initial condition
   * @param[in] function_name  names of the IC function
   */
  void addFunctionIC(const VariableName & var_name, const FunctionName & function_name);

  /**
   * Retrieves a parameter
   *
   * @param[in] name  name of the parameter
   * @return value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name) const;

  InputParameters _this_params;
  MooseApp & _this_app;
  ActionFactory & _this_action_factory;
  ActionWarehouse & _this_action_warehouse;

  MooseEnum _fe_family;
  MooseEnum _fe_order;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;
};

template <typename T>
const T &
FlowModelSetup::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _this_params, static_cast<T *>(0));
}

#endif /* FLOWMODELSETUP_H */
