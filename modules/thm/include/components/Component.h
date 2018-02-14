#ifndef COMPONENT_H
#define COMPONENT_H

#include "RELAP7App.h"
#include "RELAP7Object.h"
#include "ActionWarehouse.h"
#include "FlowModel.h"
#include "Simulation.h"
#include "InputParameterWarehouse.h"
#include "MooseError.h"

class Component;
class FEProblem;

template <>
InputParameters validParams<Component>();

/**
 * Base class for R7 components
 */
class Component : public RELAP7Object
{
public:
  Component(const InputParameters & parameters);

  unsigned int id() { return _id; }

  Component * parent() { return _parent; }

  /**
   * Initialize the component
   */
  virtual void init();

  /**
   * Perform secondary initialization, which relies on init() being called
   * for all components.
   */
  virtual void initSecondary();

  /**
   * Check the component integrity
   */
  virtual void check();

  virtual void setupMesh() = 0;

  virtual void addVariables() = 0;

  virtual void addMooseObjects() {}

  virtual const std::string & getType() = 0;

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
  bool hasComponent(const std::string & name, bool log_errors = true) const;

  /**
   * Check the existence and type of a component given its name
   * @tparam T the type of the component we are requesting
   * @param cname The name of the component
   * @return true if the component with given name and type exists, otherwise false
   */
  template <typename T>
  bool hasComponentByName(const std::string & cname, bool log_errors = true) const;

  /**
   * Connect with control logic
   */
  void connectObject(const InputParameters & params,
                     const std::string & rname,
                     const std::string & mooseName,
                     const std::string & name);
  /**
   * Connect with control logic
   */
  void connectObject(const InputParameters & params,
                     const std::string & rname,
                     const std::string & mooseName,
                     const std::string & name,
                     const std::string & par_name);

public:
  static std::string
  genName(const std::string & prefix, unsigned int id, const std::string & suffix = "");
  static std::string genName(const std::string & prefix,
                             unsigned int i,
                             unsigned int j,
                             const std::string & suffix = "");
  static std::string
  genName(const std::string & prefix, const std::string & middle, const std::string & suffix = "");

protected:
  /**
   * Makes a function controllable if it is constant
   *
   * @param[in] fn_name  name of the function
   * @param[in] control_name  name of control parameter
   * @param[in] param  name of controlled parameter
   */
  void makeFunctionControllableIfConstant(const FunctionName & fn_name,
                                          const std::string & control_name,
                                          const std::string & param = "value");

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
   */
  void checkMutuallyExclusiveParameters(const std::vector<std::string> & params) const;

  /**
   * Checks that a 1-phase parameter was provided
   *
   * @param[in] param   parameter name
   */
  void check1PhaseRequiredParameter(const std::string & param) const;

  /**
   * Checks that a 2-phase parameter was provided
   *
   * @param[in] param   parameter name
   */
  void check2PhaseRequiredParameter(const std::string & param) const;

  /**
   * Checks that a 7-equation (2-phase plus phase interaction) parameter was provided
   *
   * @param[in] param   parameter name
   */
  void check7EqnRequiredParameter(const std::string & param) const;

  /**
   * Logs an error for the model type not being implemented for the component
   *
   * @param[in] model   model type
   */
  void logModelNotImplementedError(const RELAP7::FlowModelID & model) const;

  /// Unique ID of this component
  unsigned int _id;
  /// Pointer to a parent component (used in composed components)
  Component * _parent;

  /// Simulation this component is part of
  Simulation & _sim;

  /// RELAP7 App (hides _app from MooseObject)
  RELAP7App & _app;
  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Global mesh this component works on
  RELAP7Mesh & _mesh;
  /// Global physical mesh this component works on
  RELAP7Mesh *& _phys_mesh;

  const Real & _zero;

  /// Gets the next subdomain ID
  virtual unsigned int getNextSubdomainId();

  /// Gets the next nodeset or sideset ID
  virtual unsigned int getNextBoundaryId();

  /**
   * Logs an error
   */
  template <typename... Args>
  void logError(Args &&... args) const
  {
    _app.log().add(Logger::ERROR, name(), ": ", std::forward<Args>(args)...);
  }

  /**
   * Logs a warning
   */
  template <typename... Args>
  void logWarning(Args &&... args) const
  {
    _app.log().add(Logger::WARNING, name(), ": ", std::forward<Args>(args)...);
  }

  /**
   * Split the control logic name into "section name" and "property name"
   * @param rname
   * @return
   */
  static std::vector<std::string> split(const std::string & rname);

private:
  // Do not want users to touch these, they _must_ use the API
  static unsigned int subdomain_ids;
  static unsigned int bc_ids;
};

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
Component::hasComponent(const std::string & pname, bool log_errors) const
{
  const std::string & comp_name = getParam<std::string>(pname);
  return hasComponentByName<T>(comp_name, log_errors);
}

template <typename T>
bool
Component::hasComponentByName(const std::string & comp_name, bool log_errors) const
{
  if (_sim.hasComponent(comp_name))
  {
    if (_sim.hasComponentOfType<T>(comp_name))
      return true;
    else
    {
      if (log_errors)
        logError(
            "The component '", comp_name, "' is not of type '", demangle(typeid(T).name()), "'");
      return false;
    }
  }
  else
  {
    if (log_errors)
      logError("The component '", comp_name, "' does not exist");
    return false;
  }
}

template <typename T>
void
Component::checkComponentOfTypeExists(const std::string & param) const
{
  hasComponent<T>(param, true);
}

template <typename T>
void
Component::checkComponentOfTypeExistsByName(const std::string & param) const
{
  hasComponentByName<T>(param, true);
}

template <typename T>
void
Component::checkParameterValueLessThan(const std::string & param, const T & value_max) const
{
  const auto & value = getParam<T>(param);
  if (value >= value_max)
    logError("The value of parameter '", param, "' (", value, ") must be less than ", value_max);
}

template <typename T>
void
Component::checkSizeLessThan(const std::string & param, const unsigned int & n_entries) const
{
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

#endif /* COMPONENT_H */
