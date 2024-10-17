//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <set>
#include <vector>
#include <ctime>

// MOOSE includes
#include "Registry.h"
#include "MooseObject.h"
#include "MooseTypes.h"
#include "FileLineInfo.h"

// Forward declarations
class InputParameters;

/**
 * Generic factory class for build all sorts of objects
 */
class Factory
{
public:
  Factory(MooseApp & app);
  virtual ~Factory();

  void reg(std::shared_ptr<RegistryEntryBase> obj);

  /**
   * Gets file and line information where an object was initially registered.
   * @param name Object name
   * @return The FileLineInfo associated with name
   */
  FileLineInfo getLineInfo(const std::string & name) const;

  /**
   * Associates an object name with a class name.
   * Primarily used with the registerNamed* macros to store the
   * mapping between the object name and the class that implements the object.
   */
  void associateNameToClass(const std::string & name, const std::string & class_name);

  /**
   * Get the associated class name for an object name.
   * This will return an empty string if the name was not previously
   * associated with a class name via associateNameToClass()
   */
  std::string associatedClassName(const std::string & name) const;

  /**
   * Get valid parameters for the object
   * @param name Name of the object whose parameter we are requesting
   * @return Parameters of the object
   */
  InputParameters getValidParams(const std::string & name) const;

  /**
   * Build an object (must be registered) - THIS METHOD IS DEPRECATED (Use create<T>())
   * @param obj_name Type of the object being constructed
   * @param name Name for the object
   * @param parameters Parameters this object should have
   * @param tid The thread id that this copy will be created for
   * @param print_deprecated controls the deprecated message
   * @return The created object
   */
  ///@{
  std::unique_ptr<MooseObject> createUnique(const std::string & obj_name,
                                            const std::string & name,
                                            const InputParameters & parameters,
                                            THREAD_ID tid = 0,
                                            bool print_deprecated = true);
  std::shared_ptr<MooseObject> create(const std::string & obj_name,
                                      const std::string & name,
                                      const InputParameters & parameters,
                                      THREAD_ID tid = 0,
                                      bool print_deprecated = true);
  ///@}

  /**
   * Build an object (must be registered)
   * @param obj_name Type of the object being constructed
   * @param name Name for the object
   * @param parameters Parameters this object should have
   * @param tid The thread id that this copy will be created for
   * @return The created object
   */
  ///@{
  template <typename T>
  std::unique_ptr<T> createUnique(const std::string & obj_name,
                                  const std::string & name,
                                  const InputParameters & parameters,
                                  const THREAD_ID tid = 0);
  template <typename T>
  std::shared_ptr<T> create(const std::string & obj_name,
                            const std::string & name,
                            const InputParameters & parameters,
                            const THREAD_ID tid = 0);
  ///@}

  /**
   * Clones the object \p object.
   *
   * Under the hood, this creates a copy of the InputParameters from \p object
   * and constructs a new object with the copied parameters. The suffix _clone<i>
   * will be added to the object's name, where <i> is incremented each time
   * the object is cloned.
   */
  template <typename T>
  std::unique_ptr<T> clone(const T & object);

  /**
   * Copy constructs the object \p object.
   *
   * Under the hood, the new object's parameters will point to the same address
   * as the parameters in \p object. This can be dangerous and thus this is only
   * allowed for a subset of objects.
   */
  template <typename T>
  std::unique_ptr<T> copyConstruct(const T & object);

  /**
   * Releases any shared resources created as a side effect of creating an object through
   * the Factory::create method(s). Currently, this object just moves the InputParameters object
   * from the InputParameterWarehouse. Normally this method does not need to be explicitly called
   * during a normal simulation.
   */
  void releaseSharedObjects(const MooseObject & moose_object, THREAD_ID tid = 0);

  /**
   * Calling this object with a non-empty vector will cause this factory to ignore registrations
   * from any object
   * not contained within the list.
   * @param names a vector containing the names of objects that this factory will register
   */
  void restrictRegisterableObjects(const std::vector<std::string> & names);

  /**
   * Returns a reference to the map from names to RegistryEntryBase pointers
   */
  const auto & registeredObjects() const { return _name_to_object; }

  /**
   * Returns a Boolean indicating whether an object type has been registered
   */
  bool isRegistered(const std::string & obj_name) const { return _name_to_object.count(obj_name); }

  /**
   * Get a list of all constructed Moose Object types
   */
  std::vector<std::string> getConstructedObjects() const;

  MooseApp & app() { return _app; }

  /**
   * @return The InputParameters for the object that is currently being constructed,
   * if any.
   *
   * Can be used to ensure that all MooseObjects are created using the Factory
   */
  const InputParameters * currentlyConstructing() const;

private:
  /**
   * Parse time string (mm/dd/yyyy HH:MM)
   * @param t_str String with the object expiration date, this must be in the form mm/dd/yyyy
   * HH:MM
   * @return A time_t object with the expiration date
   */
  std::time_t parseTime(std::string);

  /**
   * Show the appropriate message for deprecated objects
   * @param obj_name Name of the deprecated object
   */
  void deprecatedMessage(const std::string obj_name) const;

  /**
   * Prints error information when an object is not registered
   */
  void reportUnregisteredError(const std::string & obj_name) const;

  /**
   * Initializes the data structures and the parameters (in the InputParameterWarehouse)
   * for the object with the given state.
   */
  InputParameters & initialize(const std::string & type,
                               const std::string & name,
                               const InputParameters & from_params,
                               const THREAD_ID tid);

  /**
   * Finalizes the creaction of \p object of type \p type.
   *
   * This will do some sanity checking on whether or not the parameters in the
   * created object match the valid paramters of the associated type.
   */
  void finalize(const std::string & type, const MooseObject & object);

  /// Reference to the application
  MooseApp & _app;

  /// Storage for pointers to the object registry entry
  std::map<std::string, std::shared_ptr<RegistryEntryBase>> _name_to_object;

  FileLineInfoMap _name_to_line;

  /// Object name to class name association
  std::map<std::string, std::string> _name_to_class;

  /// Storage for deprecated object experiation dates
  std::map<std::string, std::time_t> _deprecated_time;

  /// Storage for the deprecated objects that have replacements
  std::map<std::string, std::string> _deprecated_name;

  /// The list of objects that may be registered
  std::set<std::string> _registerable_objects;

  /// Constructed Moose Object types
  std::set<std::string> _constructed_types;

  /// Set of deprecated object types that have been printed
  mutable std::set<std::string> _deprecated_types;

  /// set<label/appname, objectname> used to track if an object previously added is being added
  /// again - which is okay/allowed, while still allowing us to detect/reject cases of duplicate
  /// object name registration where the label/appname is not identical.
  std::set<std::pair<std::string, std::string>> _objects_by_label;

  /// The object's parameters that are currently being constructed (if any).
  /// This is a vector because we create within create, thus the last entry is the
  /// one that is being constructed at the moment
  std::vector<const InputParameters *> _currently_constructing;

  /// Counter for keeping track of the number of times an object with a given name has
  /// been cloned so that we can continue to create objects with unique names
  std::map<const MooseObject *, unsigned int> _clone_counter;
};

template <typename T>
std::unique_ptr<T>
Factory::createUnique(const std::string & obj_name,
                      const std::string & name,
                      const InputParameters & parameters,
                      const THREAD_ID tid)
{
  auto object = createUnique(obj_name, name, parameters, tid, false);
  if (!dynamic_cast<T *>(object.get()))
    mooseError("We expected to create an object of type '" + libMesh::demangle(typeid(T).name()) +
               "'.\nInstead we received a parameters object for type '" + obj_name +
               "'.\nDid you call the wrong \"add\" method in your Action?");

  return std::unique_ptr<T>(static_cast<T *>(object.release()));
}

template <typename T>
std::shared_ptr<T>
Factory::create(const std::string & obj_name,
                const std::string & name,
                const InputParameters & parameters,
                const THREAD_ID tid)
{
  return std::move(createUnique<T>(obj_name, name, parameters, tid));
}

template <typename T>
std::unique_ptr<T>
Factory::clone(const T & object)
{
  static_assert(std::is_base_of_v<MooseObject, T>, "Not a MooseObject");

  const auto tid = object.template getParam<THREAD_ID>("_tid");
  if (tid != 0)
    mooseError("Factory::clone(): The object ",
               object.typeAndName(),
               " is threaded but cloning does not work with threaded objects");

  // Clone the parameters; we can't copy construct InputParameters
  InputParameters cloned_params = emptyInputParameters();
  cloned_params += object.parameters();
  if (const auto hit_node = object.parameters().getHitNode())
    cloned_params.setHitNode(*hit_node, {});

  // Fill the new parameters in the warehouse
  const auto type = static_cast<const MooseBase &>(object).type();
  const auto clone_count = _clone_counter[&object]++;
  const auto name = object.name() + "_clone" + std::to_string(clone_count);
  const auto & params = initialize(type, name, cloned_params, 0);

  // Construct the object
  _currently_constructing.push_back(&params);
  auto cloned_object = std::make_unique<T>(params);
  _currently_constructing.pop_back();

  // Do some sanity checking
  finalize(type, *cloned_object);

  return cloned_object;
}

template <typename T>
std::unique_ptr<T>
Factory::copyConstruct(const T & object)
{
  static_assert(std::is_base_of_v<MooseObject, T>, "Not a MooseObject");

  const auto type = static_cast<const MooseBase &>(object).type();
  const auto base = object.parameters().getBase();
  if (!base || (*base != "MooseMesh" && *base != "RelationshipManager"))
    mooseError("Copy construction of ", type, " objects is not supported.");

  _currently_constructing.push_back(&object.parameters());
  auto cloned_object = std::make_unique<T>(object);
  _currently_constructing.pop_back();

  finalize(type, *cloned_object);

  return cloned_object;
}
