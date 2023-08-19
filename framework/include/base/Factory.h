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
  InputParameters getValidParams(const std::string & name);

  /**
   * Build an object (must be registered) - THIS METHOD IS DEPRECATED (Use create<T>())
   * @param obj_name Type of the object being constructed
   * @param name Name for the object
   * @param parameters Parameters this object should have
   * @param tid The thread id that this copy will be created for
   * @param print_deprecated controls the deprecated message
   * @return The created object
   */
  std::shared_ptr<MooseObject> create(const std::string & obj_name,
                                      const std::string & name,
                                      const InputParameters & parameters,
                                      THREAD_ID tid = 0,
                                      bool print_deprecated = true);

  /**
   * Build an object (must be registered)
   * @param obj_name Type of the object being constructed
   * @param name Name for the object
   * @param parameters Parameters this object should have
   * @param tid The thread id that this copy will be created for
   * @return The created object
   */
  template <typename T>
  std::shared_ptr<T> create(const std::string & obj_name,
                            const std::string & name,
                            const InputParameters & parameters,
                            THREAD_ID tid = 0)
  {
    std::shared_ptr<T> new_object =
        std::dynamic_pointer_cast<T>(create(obj_name, name, parameters, tid, false));
    if (!new_object)
      mooseError("We expected to create an object of type '" + demangle(typeid(T).name()) +
                 "'.\nInstead we received a parameters object for type '" + obj_name +
                 "'.\nDid you call the wrong \"add\" method in your Action?");

    return new_object;
  }

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
  void deprecatedMessage(const std::string obj_name);

  /**
   * Prints error information when an object is not registered
   */
  void reportUnregisteredError(const std::string & obj_name) const;

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
  std::set<std::string> _deprecated_types;

  /// set<label/appname, objectname> used to track if an object previously added is being added
  /// again - which is okay/allowed, while still allowing us to detect/reject cases of duplicate
  /// object name registration where the label/appname is not identical.
  std::set<std::pair<std::string, std::string>> _objects_by_label;
};
