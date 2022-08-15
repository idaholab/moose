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
#include "MooseObject.h"
#include "MooseTypes.h"
#include "FileLineInfo.h"

// Forward declarations
class InputParameters;

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerObject(name) factory.reg<name>(stringifyName(name), __FILE__, __LINE__)
#define registerNamedObject(obj, name)                                                             \
  do                                                                                               \
  {                                                                                                \
    factory.reg<obj>(name, __FILE__, __LINE__);                                                    \
    factory.associateNameToClass(name, stringifyName(obj));                                        \
  } while (0)

#define registerDeprecatedObject(name, time)                                                       \
  factory.regDeprecated<name>(stringifyName(name), time, __FILE__, __LINE__)

#define registerDeprecatedObjectWithReplacement(dep_obj, replacement_name, time)                   \
  factory.regReplaced<dep_obj>(stringifyName(dep_obj), replacement_name, time, __FILE__, __LINE__)

#define registerRenamedObject(orig_name, new_obj, time)                                            \
  factory.renameObject<new_obj>(orig_name, stringifyName(new_obj), time, __FILE__, __LINE__)

// for backward compatibility
#define registerKernel(name) registerObject(name)
#define registerNodalKernel(name) registerObject(name)
#define registerBoundaryCondition(name) registerObject(name)
#define registerAux(name) registerObject(name)
#define registerAuxKernel(name) registerObject(name)
#define registerMaterial(name) registerObject(name)
#define registerPostprocessor(name) registerObject(name)
#define registerVectorPostprocessor(name) registerObject(name)
#define registerInitialCondition(name) registerObject(name)
#define registerDamper(name) registerObject(name)
#define registerDiracKernel(name) registerObject(name)
#define registerDGKernel(name) registerObject(name)
#define registerInterfaceKernel(name) registerObject(name)
#define registerExecutioner(name) registerObject(name)
#define registerFunction(name) registerObject(name)
#define registerDistribution(name) registerObject(name)
#define registerSampler(name) registerObject(name)
#define registerMesh(name) registerObject(name)
#define registerConstraint(name) registerObject(name)
#define registerScalarKernel(name) registerObject(name)
#define registerUserObject(name) registerObject(name)
#define registerPreconditioner(name) registerObject(name)
#define registerIndicator(name) registerObject(name)
#define registerMarker(name) registerObject(name)
#define registerProblem(name) registerObject(name)
#define registerMultiApp(name) registerObject(name)
#define registerTransfer(name) registerObject(name)
#define registerTimeStepper(name) registerObject(name)
#define registerTimeIntegrator(name) registerObject(name)
#define registerPredictor(name) registerObject(name)
#define registerSplit(name) registerObject(name)
#define registerOutput(name) registerObject(name)
#define registerControl(name) registerObject(name)
#define registerPartitioner(name) registerObject(name)
#define registerRelationshipManager(name) registerObject(name)

#define registerNamedKernel(obj, name) registerNamedObject(obj, name)
#define registerNamedNodalKernel(obj, name) registerNamedObject(obj, name)
#define registerNamedBoundaryCondition(obj, name) registerNamedObject(obj, name)
#define registerNamedAux(obj, name) registerNamedObject(obj, name)
#define registerNamedAuxKernel(name) registerNamedObject(obj, name)
#define registerNamedMaterial(obj, name) registerNamedObject(obj, name)
#define registerNamedPostprocessor(obj, name) registerNamedObject(obj, name)
#define registerNamedVectorPostprocessor(obj, name) registerNamedObject(obj, name)
#define registerNamedInitialCondition(obj, name) registerNamedObject(obj, name)
#define registerNamedDamper(obj, name) registerNamedObject(obj, name)
#define registerNamedDiracKernel(obj, name) registerNamedObject(obj, name)
#define registerNamedDGKernel(obj, name) registerNamedObject(obj, name)
#define registerNamedExecutioner(obj, name) registerNamedObject(obj, name)
#define registerNamedFunction(obj, name) registerNamedObject(obj, name)
#define registerNamedDistribution(obj, name) registerNamedObject(obj, name)
#define registerNamedSampler(obj, name) registerNamedObject(obj, name)
#define registerNamedMesh(obj, name) registerNamedObject(obj, name)
#define registerNamedConstraint(obj, name) registerNamedObject(obj, name)
#define registerNamedUserObject(obj, name) registerNamedObject(obj, name)
#define registerNamedPreconditioner(obj, name) registerNamedObject(obj, name)
#define registerNamedIndicator(obj, name) registerNamedObject(obj, name)
#define registerNamedMarker(obj, name) registerNamedObject(obj, name)
#define registerNamedProblem(obj, name) registerNamedObject(obj, name)
#define registerNamedMultiApp(obj, name) registerNamedObject(obj, name)
#define registerNamedTransfer(obj, name) registerNamedObject(obj, name)
#define registerNamedTimeStepper(obj, name) registerNamedObject(obj, name)
#define registerNamedTimeIntegrator(obj, name) registerNamedObject(obj, name)
#define registerNamedPredictor(obj, name) registerNamedObject(obj, name)
#define registerNamedSplit(obj, name) registerNamedObject(obj, name)
#define registerNamedOutput(obj, name) registerNamedObject(obj, name)
#define registerNamedControl(obj, name) registerNamedObject(obj, name)
#define registerNamedPartitioner(obj, name) registerNamedObject(obj, name);

/**
 * alias to wrap shared pointer type
 */
using MooseObjectPtr = std::shared_ptr<MooseObject>;

/**
 * alias for validParams function
 */
using paramsPtr = InputParameters (*)();

/**
 * alias for method to build objects
 */
using buildPtr = MooseObjectPtr (*)(const InputParameters & parameters);

/**
 * alias for registered Object iterator
 */
using registeredMooseObjectIterator = std::map<std::string, paramsPtr>::iterator;

/**
 * Generic factory class for build all sorts of objects
 */
class Factory
{
public:
  Factory(MooseApp & app);
  virtual ~Factory();

  /**
   * Register a new object
   * @param obj_name Name of the object to register
   */
  template <typename T>
  void reg(const std::string & obj_name, const std::string & file = "", int line = -1)
  {
    reg("", obj_name, &buildObject<T>, &moose::internal::callValidParams<T>, "", "", file, line);
  }

  void reg(const std::string & label,
           const std::string & obj_name,
           const buildPtr & build_ptr,
           const paramsPtr & params_ptr,
           const std::string & deprecated_time = "",
           const std::string & replacement_name = "",
           const std::string & file = "",
           int line = -1);

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
   * Register a deprecated object that expires
   * @param obj_name The name of the object to register
   * @param t_str String containing the expiration date for the object in "MM/DD/YYYY HH:MM"
   * format. Note that the HH:MM is not optional
   *
   * Note: Params file and line are supplied by the macro
   */
  template <typename T>
  void regDeprecated(const std::string & obj_name,
                     const std::string t_str,
                     const std::string & file,
                     int line)
  {
    reg("", obj_name, &buildObject<T>, &moose::internal::callValidParams<T>, t_str, "", file, line);
  }

  /**
   * Registers an object as deprecated and associates it with the replacement name.
   * @param dep_obj - The name (type) of the object being registered (the deprecated type)
   * @param replacement_name - The name of the object replacing the deprecated object (new name)
   * @param time_str - Time at which the deprecated message prints as  an error "MM/DD/YYYY HH:MM"
   * Note that the HH:MM is not optional
   *
   * Note: Params file and line are supplied by the macro
   */
  template <typename T>
  void regReplaced(const std::string & dep_obj,
                   const std::string & replacement_name,
                   const std::string time_str,
                   const std::string & file,
                   int line)
  {
    reg("",
        dep_obj,
        &buildObject<T>,
        &moose::internal::callValidParams<T>,
        time_str,
        replacement_name,
        file,
        line);
  }

  /**
   * Used when an existing object's name changes
   *
   * Template T: The type of the new class
   *
   * @param orig_name The name of the original class
   * @param new_name The name of the new class
   * @param time_str The date the deprecation will expire
   *
   * Note: Params file and line are supplied by the macro
   */
  template <typename T>
  void renameObject(const std::string & orig_name,
                    const std::string & new_name,
                    const std::string time_str,
                    const std::string & file,
                    int line)
  {
    // Deprecate the old name
    // Store the time
    _deprecated_time[orig_name] = parseTime(time_str);

    // Store the new name
    _deprecated_name[orig_name] = new_name;

    // Register the new object with the old name
    reg<T>(orig_name, __FILE__, __LINE__);
    associateNameToClass(orig_name, new_name);

    // Register the new object with the new name
    reg<T>(new_name, file, line);
  }

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
   * Access to registered object iterator (begin)
   */
  registeredMooseObjectIterator registeredObjectsBegin() { return _name_to_params_pointer.begin(); }

  /**
   * Access to registered object iterator (end)
   */
  registeredMooseObjectIterator registeredObjectsEnd() { return _name_to_params_pointer.end(); }

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

  /**
   * Build an object of type T
   */
  template <typename T>
  static MooseObjectPtr buildObject(const InputParameters & parameters)
  {
    return std::make_shared<T>(parameters);
  }

  /// Reference to the application
  MooseApp & _app;

  /// Storage for pointers to the object
  std::map<std::string, buildPtr> _name_to_build_pointer;

  /// Storage for pointers to the parameters objects
  std::map<std::string, paramsPtr> _name_to_params_pointer;

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

  /// set<label/appname, objectname> used to track if an object previously added is being added
  /// again - which is okay/allowed, while still allowing us to detect/reject cases of duplicate
  /// object name registration where the label/appname is not identical.
  std::set<std::pair<std::string, std::string>> _objects_by_label;
};
