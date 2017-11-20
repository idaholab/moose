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

  virtual const std::vector<unsigned int> & getSubdomainIds() const { return _subdomains; }

  virtual const std::vector<Moose::CoordinateSystemType> & getCoordSysTypes() { return _coord_sys; }

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

  /// List of subdomain IDs this components owns
  std::vector<unsigned int> _subdomains;
  /// List of coordinate system per subdomain (the same length as _subdomains and items correspond to _subdomains entries)
  std::vector<Moose::CoordinateSystemType> _coord_sys;

  const Real & _zero;

  virtual unsigned int getNextSubdomainId();

  /// Gets the next nodeset or sideset ID
  virtual unsigned int getNextBoundaryId();

  /// Sets the coordinate system for block_id sudomain
  virtual void setSubdomainCoordSystem(unsigned int block_id,
                                       Moose::CoordinateSystemType coord_type);

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
        logError("Requested component '",
                 comp_name,
                 "' has to be of type ",
                 demangle(typeid(T).name()),
                 ".");
      return false;
    }
  }
  else
  {
    if (log_errors)
      logError("Requesting a non-existing component '", comp_name, "'. Typo?");
    return false;
  }
}

#endif /* COMPONENT_H */
