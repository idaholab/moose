//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMObject.h"
#include "FlowModel.h"
#include "THMProblem.h"
#include "InputParameterWarehouse.h"
#include "LoggingInterface.h"
#include "NamingInterface.h"

class THMProblem;
class THMMesh;
class ThermalHydraulicsApp;

/**
 * Base class for THM components
 */
class Component : public THMObject, public LoggingInterface, public NamingInterface
{
public:
  Component(const InputParameters & parameters);

  /// Component setup status type
  enum EComponentSetupStatus
  {
    CREATED,                  ///< only created
    PRE_SETUP_MESH_COMPLETED, ///< preSetupMesh() executed
    MESH_PREPARED,            ///< mesh set up
    INITIALIZED_PRIMARY,      ///< mesh set up, called primary init
    INITIALIZED_SECONDARY,    ///< mesh set up, called both inits
    CHECKED                   ///< mesh set up, called both inits, checked
  };

  /**
   * Get the component name
   * @return The name of the component. For composite component, return its parent name
   */
  const std::string & cname() const;

  Component * parent() { return _parent; }

  /**
   * Const reference to mesh, which can be called at any point
   *
   * Note that overloading mesh() was not possible due to the need to call this
   * const version, even when the component is not const.
   */
  const THMMesh & constMesh() const { return _mesh; }

  /**
   * Non-const reference to THM mesh, which can only be called before the end of mesh setup
   */
  THMMesh & mesh();

  /**
   * Gets the THM problem
   */
  THMProblem & getTHMProblem() const;

  /**
   * Test if a parameter exists in the object's input parameters
   * @param name The name of the parameter
   * @return true if the parameter exists, false otherwise
   */
  template <typename T>
  bool hasParam(const std::string & name) const;

  /**
   * Returns a list of names of components that this component depends upon
   */
  const std::vector<std::string> & getDependencies() const { return _dependencies; }

  /**
   * Wrapper function for \c preSetupMesh() that marks the function as being called
   */
  void executePreSetupMesh();

  /**
   * Wrapper function for \c init() that marks the function as being called
   */
  void executeInit();

  /**
   * Wrapper function for \c initSecondary() that marks the function as being called
   */
  void executeInitSecondary();

  /**
   * Wrapper function for \c check() that marks the function as being called
   */
  void executeCheck() const;

  /**
   * Wrapper function for \c setupMesh() that marks the function as being called
   */
  void executeSetupMesh();

  /**
   * Adds relationship managers for the component
   */
  virtual void addRelationshipManagers(Moose::RelationshipManagerType /*input_rm_type*/) {}

  virtual void addVariables() {}

  virtual void addMooseObjects() {}

  /**
   * Return a reference to a component via a parameter name
   * @tparam T the type of the component we are requesting
   * @param name The parameter name that has the component name
   */
  template <typename T>
  const T & getComponent(const std::string & name) const;

  /**
   * Return a reference to a component given its name
   * @tparam T the type of the component we are requesting
   * @param cname The name of the component
   */
  template <typename T>
  const T & getComponentByName(const std::string & cname) const;

  /**
   * Check the existence and type of a component via a parameter name
   * @tparam T the type of the component we are requesting
   * @param name The parameter name that has the component name
   * @return true if the component with given name and type exists, otherwise false
   */
  template <typename T>
  bool hasComponent(const std::string & name) const;

  /**
   * Check the existence and type of a component given its name
   * @tparam T the type of the component we are requesting
   * @param cname The name of the component
   * @return true if the component with given name and type exists, otherwise false
   */
  template <typename T>
  bool hasComponentByName(const std::string & cname) const;

  /**
   * Connect with control logic
   */
  void connectObject(const InputParameters & params,
                     const std::string & mooseName,
                     const std::string & name) const;
  /**
   * Connect with control logic
   */
  void connectObject(const InputParameters & params,
                     const std::string & mooseName,
                     const std::string & name,
                     const std::string & par_name) const;

  /**
   * Makes a function controllable if it is constant
   *
   * @param[in] fn_name  name of the function
   * @param[in] control_name  name of control parameter
   * @param[in] param  name of controlled parameter
   */
  void makeFunctionControllableIfConstant(const FunctionName & fn_name,
                                          const std::string & control_name,
                                          const std::string & param = "value") const;

  /**
   * Throws an error if the supplied setup status of this component has not been reached
   *
   * This is useful for getter functions that rely on data initialized after the
   * constructor; if an error is not thrown, then uninitialized data could be
   * returned from these functions.
   *
   * @param[in] status   Setup status that this component must have reached
   */
  void checkSetupStatus(const EComponentSetupStatus & status) const;

  /**
   * Checks that a component exists
   *
   * @param[in] comp_name   name of the component
   */
  void checkComponentExistsByName(const std::string & comp_name) const;

  /**
   * Checks that the component of a certain type exists, where the name is given by a parameter
   *
   * @tparam    T       enforced type of component
   * @param[in] param   parameter name for component name
   */
  template <typename T>
  void checkComponentOfTypeExists(const std::string & param) const;

  /**
   * Checks that the component of a certain type exists
   *
   * @tparam    T           enforced type of component
   * @param[in] comp_name   component name
   */
  template <typename T>
  void checkComponentOfTypeExistsByName(const std::string & comp_name) const;

  /**
   * Logs an error
   */
  template <typename... Args>
  void logError(Args &&... args) const
  {
    logComponentError(cname(), std::forward<Args>(args)...);
  }

  /**
   * Logs a warning
   */
  template <typename... Args>
  void logWarning(Args &&... args) const
  {
    logComponentWarning(cname(), std::forward<Args>(args)...);
  }

  /**
   * Adds a component name to the list of dependencies
   *
   * @param[in] dependency   name of component to add to list of dependencies
   */
  void addDependency(const std::string & dependency);

  /**
   * Gets an enum parameter
   *
   * This function takes the name of a MooseEnum parameter that is tied to an
   * enum defined in THM. If the value is invalid, an error will be logged,
   * and a negative integer will be cast into the enum type.
   *
   * @tparam    T       enum type
   * @param[in] param   name of the MooseEnum parameter
   */
  template <typename T>
  T getEnumParam(const std::string & param) const;

protected:
  /**
   * Performs any post-constructor, pre-mesh-setup setup
   */
  virtual void preSetupMesh() {}

  /**
   * Initializes the component
   *
   * The reason this function exists (as opposed to just having everything in
   * the constructor) is because some initialization depends on all components
   * existing, since many components couple to other components. Therefore,
   * when deciding whether code should go into the constructor or this function,
   * one should use the following reasoning: if an operation does not require
   * the existence of other components, then put that operation in the
   * constructor; otherwise, put it in this function.
   */
  virtual void init() {}

  /**
   * Perform secondary initialization, which relies on init() being called
   * for all components.
   */
  virtual void initSecondary() {}

  /**
   * Check the component integrity
   */
  virtual void check() const {}

  /**
   * Performs mesh setup such as creating mesh or naming mesh sets
   */
  virtual void setupMesh() {}

  /**
   * Method to add a relationship manager for the objects being added to the system. Relationship
   * managers have to be added relatively early. In many cases before the Action::act() method
   * is called.
   *
   * This method was copied from Action.
   *
   * @param moose_object_pars The MooseObject to inspect for RelationshipManagers to add
   */
  void addRelationshipManagersFromParameters(const InputParameters & moose_object_pars);

  /**
   * Runtime check to make sure that a parameter of specified type exists in the component's input
   * parameters
   *
   * This is intended to help developers write code.  The idea is to provide a useful message when
   * developers make typos, etc.  If this check fails, the code execution will be stopped.
   *
   * @tparam T The type of the parameter to be checked
   * @param function_name The name of the function calling this method
   * @param param_name The name of the parameter to be checked
   */
  template <typename T>
  void insistParameterExists(const std::string & function_name,
                             const std::string & param_name) const;

  /**
   * Checks that a parameter value is less than a value
   *
   * @tparam    T           type of parameter
   * @param[in] param       parameter name
   * @param[in] value_max   value which parameter value must be less than
   */
  template <typename T>
  void checkParameterValueLessThan(const std::string & param, const T & value_max) const;

  /**
   * Checks that the size of a vector parameter is less than a value
   *
   * @tparam    T           type of element in the vector parameter
   * @param[in] param       parameter name
   * @param[in] n_entries   value which parameter size must be less than
   */
  template <typename T>
  void checkSizeLessThan(const std::string & param, const unsigned int & n_entries) const;

  /**
   * Checks that the size of a vector parameter is greater than a value
   *
   * @tparam    T           type of element in the vector parameter
   * @param[in] param       parameter name
   * @param[in] n_entries   value which parameter size must be greater than
   */
  template <typename T>
  void checkSizeGreaterThan(const std::string & param, const unsigned int & n_entries) const;

  /**
   * Checks that the size of two vector parameters are equal
   *
   * @tparam    T1       type of element in the first vector parameter
   * @tparam    T2       type of element in the second vector parameter
   * @param[in] param1   first parameter name
   * @param[in] param2   second parameter name
   */
  template <typename T1, typename T2>
  void checkEqualSize(const std::string & param1, const std::string & param2) const;

  /**
   * Checks that the size of a vector parameter equals a value
   *
   * This version does not supply a description to the value.
   *
   * @tparam    T           type of element in the vector parameter
   * @param[in] param       parameter name
   * @param[in] n_entries   value which parameter size must be equal to
   */
  template <typename T>
  void checkSizeEqualsValue(const std::string & param, const unsigned int & n_entries) const;

  /**
   * Checks that the size of a vector parameter equals a value
   *
   * This version supplies a description to the value.
   *
   * @tparam    T             type of element in the vector parameter
   * @param[in] param         parameter name
   * @param[in] n_entries     value which parameter size must be equal to
   * @param[in] description   description of the value that size must be equal to
   */
  template <typename T>
  void checkSizeEqualsValue(const std::string & param,
                            const unsigned int & n_entries,
                            const std::string & description) const;

  /**
   * Checks that the size of a vector parameter equals the value of another parameter
   *
   * @tparam    T1       type of element in the vector parameter
   * @tparam    T2       type of the parameter whose value is compared to size
   * @param[in] param1   vector parameter name
   * @param[in] param2   name of parameter whose value is compared to size
   */
  template <typename T1, typename T2>
  void checkSizeEqualsParameterValue(const std::string & param1, const std::string & param2) const;

  /**
   * Checks that exactly one parameter out of a list is provided
   *
   * @param[in] params   vector of parameter names
   * @param[in] need_one_specified   Need one of the parameters specified?
   */
  void checkMutuallyExclusiveParameters(const std::vector<std::string> & params,
                                        bool need_one_specified = true) const;

  /// Pointer to a parent component (used in composed components)
  Component * _parent;

  /// THM problem this component is part of
  /// TODO: make _sim private (applications need to switch to getters to avoid breaking).
  /// Also, rename to "_thm_problem" at that point.
  THMProblem & _sim;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  const Real & _zero;

  /// The THM mesh
  /// TODO: make _mesh private (applications need to switch to getters to avoid breaking)
  THMMesh & _mesh;

private:
  /**
   * Method for adding a single relationship manager
   *
   * This method was copied from Action.
   *
   * @param moose_object_pars The parameters of the MooseObject that requested the RM
   * @param rm_name The class type of the RM, e.g. ElementSideNeighborLayers
   * @param rm_type The RelationshipManagerType, e.g. geometric, algebraic, coupling
   * @param rm_input_parameter_func The RM callback function, typically a lambda defined in the
   *                                requesting MooseObject's validParams function
   * @param sys_type A RMSystemType that can be used to limit the systems and consequent dof_maps
   *                 that the RM can be attached to
   */
  void
  addRelationshipManager(const InputParameters & moose_object_pars,
                         std::string rm_name,
                         Moose::RelationshipManagerType rm_type,
                         Moose::RelationshipManagerInputParameterCallback rm_input_parameter_func,
                         Moose::RMSystemType sys_type = Moose::RMSystemType::NONE);

  /// Component setup status
  mutable EComponentSetupStatus _component_setup_status;

  /// List of names of components that this component depends upon
  std::vector<std::string> _dependencies;

public:
  static InputParameters validParams();
};

template <typename T>
bool
Component::hasParam(const std::string & name) const
{
  return parameters().have_parameter<T>(name);
}

template <typename T>
const T &
Component::getComponent(const std::string & pname) const
{
  const std::string & comp_name = getParam<std::string>(pname);
  return getComponentByName<T>(comp_name);
}

template <typename T>
const T &
Component::getComponentByName(const std::string & comp_name) const
{
  return _sim.getComponentByName<T>(comp_name);
}

template <typename T>
bool
Component::hasComponent(const std::string & pname) const
{
  const std::string & comp_name = getParam<std::string>(pname);
  return hasComponentByName<T>(comp_name);
}

template <typename T>
bool
Component::hasComponentByName(const std::string & comp_name) const
{
  if (_sim.hasComponentOfType<T>(comp_name))
    return true;
  else
    return false;
}

template <typename T>
T
Component::getEnumParam(const std::string & param) const
{
  const MooseEnum & moose_enum = getParam<MooseEnum>(param);
  const T value = THM::stringToEnum<T>(moose_enum);
  if (static_cast<int>(value) < 0) // cast necessary for scoped enums
  {
    // Get the keys from the MooseEnum. Unfortunately, this returns a list of
    // *all* keys, including the invalid key that was supplied. Thus, that key
    // needs to be manually excluded below.
    const std::vector<std::string> & keys = moose_enum.getNames();

    // Create the string of keys to go in the error message. The last element of
    // keys is skipped because the invalid key should always be last.
    std::string keys_string = "{";
    for (unsigned int i = 0; i < keys.size() - 1; ++i)
    {
      if (i != 0)
        keys_string += ",";
      keys_string += "'" + keys[i] + "'";
    }
    keys_string += "}";

    logError("The parameter '" + param + "' was given an invalid value ('" +
             std::string(moose_enum) + "'). Valid values (case-insensitive) are " + keys_string);
  }

  return value;
}

template <typename T>
void
Component::insistParameterExists(const std::string & function_name,
                                 const std::string & param_name) const
{
  if (!hasParam<T>(param_name))
    mooseError(name(),
               ": Calling ",
               function_name,
               " failed, parameter '",
               param_name,
               "' does not exist or does not have the type you requested. Double check your "
               "spelling and/or type of the parameter.");
}

template <typename T>
void
Component::checkComponentOfTypeExists(const std::string & param) const
{
  insistParameterExists<std::string>(__FUNCTION__, param);

  const std::string & comp_name = getParam<std::string>(param);
  checkComponentOfTypeExistsByName<T>(comp_name);
}

template <typename T>
void
Component::checkComponentOfTypeExistsByName(const std::string & comp_name) const
{
  if (!_sim.hasComponentOfType<T>(comp_name))
  {
    if (_sim.hasComponent(comp_name))
      logError("The component '", comp_name, "' is not of type '", demangle(typeid(T).name()), "'");
    else
      logError("The component '", comp_name, "' does not exist");
  }
}

template <typename T>
void
Component::checkParameterValueLessThan(const std::string & param, const T & value_max) const
{
  insistParameterExists<T>(__FUNCTION__, param);

  const auto & value = getParam<T>(param);
  if (value >= value_max)
    logError("The value of parameter '", param, "' (", value, ") must be less than ", value_max);
}

template <typename T>
void
Component::checkSizeLessThan(const std::string & param, const unsigned int & n_entries) const
{
  insistParameterExists<std::vector<T>>(__FUNCTION__, param);

  const auto & value = getParam<std::vector<T>>(param);
  if (value.size() >= n_entries)
    logError("The number of entries in the parameter '",
             param,
             "' (",
             value.size(),
             ") must be less than ",
             n_entries);
}

template <typename T>
void
Component::checkSizeGreaterThan(const std::string & param, const unsigned int & n_entries) const
{
  insistParameterExists<std::vector<T>>(__FUNCTION__, param);

  const auto & value = getParam<std::vector<T>>(param);
  if (value.size() <= n_entries)
    logError("The number of entries in the parameter '",
             param,
             "' (",
             value.size(),
             ") must be greater than ",
             n_entries);
}

template <typename T1, typename T2>
void
Component::checkEqualSize(const std::string & param1, const std::string & param2) const
{
  insistParameterExists<std::vector<T1>>(__FUNCTION__, param1);
  insistParameterExists<std::vector<T2>>(__FUNCTION__, param2);

  const auto & value1 = getParam<std::vector<T1>>(param1);
  const auto & value2 = getParam<std::vector<T2>>(param2);
  if (value1.size() != value2.size())
    logError("The number of entries in parameter '",
             param1,
             "' (",
             value1.size(),
             ") must equal the number of entries of parameter '",
             param2,
             "' (",
             value2.size(),
             ")");
}

template <typename T>
void
Component::checkSizeEqualsValue(const std::string & param, const unsigned int & n_entries) const
{
  insistParameterExists<std::vector<T>>(__FUNCTION__, param);

  const auto & param_value = getParam<std::vector<T>>(param);
  if (param_value.size() != n_entries)
    logError("The number of entries in parameter '",
             param,
             "' (",
             param_value.size(),
             ") must be equal to ",
             n_entries);
}

template <typename T>
void
Component::checkSizeEqualsValue(const std::string & param,
                                const unsigned int & n_entries,
                                const std::string & description) const
{
  insistParameterExists<std::vector<T>>(__FUNCTION__, param);

  const auto & param_value = getParam<std::vector<T>>(param);
  if (param_value.size() != n_entries)
    logError("The number of entries in parameter '",
             param,
             "' (",
             param_value.size(),
             ") must be equal to ",
             description,
             " (",
             n_entries,
             ")");
}

template <typename T1, typename T2>
void
Component::checkSizeEqualsParameterValue(const std::string & param1,
                                         const std::string & param2) const
{
  insistParameterExists<std::vector<T1>>(__FUNCTION__, param1);
  insistParameterExists<T2>(__FUNCTION__, param2);

  const auto & value1 = getParam<std::vector<T1>>(param1);
  const auto & value2 = getParam<T2>(param2);
  if (value1.size() != value2)
    logError("The number of entries in parameter '",
             param1,
             "' (",
             value1.size(),
             ") must be equal to the value of parameter '",
             param2,
             "' (",
             value2,
             ")");
}
