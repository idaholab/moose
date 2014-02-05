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

#ifndef FACTORY_H
#define FACTORY_H

#include <vector>
#include <time.h>

#include "MooseObject.h"
#include "InputParameters.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerObject(name)                        factory.reg<name>(stringifyName(name))
#define registerNamedObject(obj, name)              factory.reg<obj>(name)

// for backward compatibility
#define registerKernel(name)                        registerObject(name)
#define registerBoundaryCondition(name)             registerObject(name)
#define registerAux(name)                           registerObject(name)
#define registerMaterial(name)                      registerObject(name)
#define registerPostprocessor(name)                 registerObject(name)
#define registerInitialCondition(name)              registerObject(name)
#define registerDamper(name)                        registerObject(name)
#define registerDiracKernel(name)                   registerObject(name)
#define registerDGKernel(name)                      registerObject(name)
#define registerExecutioner(name)                   registerObject(name)
#define registerFunction(name)                      registerObject(name)
#define registerMesh(name)                          registerObject(name)
#define registerMeshModifier(name)                  registerObject(name)
#define registerConstraint(name)                    registerObject(name)
#define registerScalarKernel(name)                  registerObject(name)
#define registerUserObject(name)                    registerObject(name)
#define registerPreconditioner(name)                registerObject(name)
#define registerIndicator(name)                     registerObject(name)
#define registerMarker(name)                        registerObject(name)
#define registerProblem(name)                       registerObject(name)
#define registerMultiApp(name)                      registerObject(name)
#define registerTransfer(name)                      registerObject(name)
#define registerTimeStepper(name)                   registerObject(name)
#define registerTimeIntegrator(name)                registerObject(name)
#define registerPredictor(name)                     registerObject(name)
#define registerSplit(name)                         registerObject(name)
#define registerOutput(name)                        registerObject(name)


#define registerNamedKernel(obj, name)              registerNamedObject(obj, name)
#define registerNamedBoundaryCondition(obj, name)   registerNamedObject(obj, name)
#define registerNamedAux(obj, name)                 registerNamedObject(obj, name)
#define registerNamedMaterial(obj, name)            registerNamedObject(obj, name)
#define registerNamedPostprocessor(obj, name)       registerNamedObject(obj, name)
#define registerNamedInitialCondition(obj, name)    registerNamedObject(obj, name)
#define registerNamedDamper(obj, name)              registerNamedObject(obj, name)
#define registerNamedDiracKernel(obj, name)         registerNamedObject(obj, name)
#define registerNamedDGKernel(obj, name)            registerNamedObject(obj, name)
#define registerNamedExecutioner(obj, name)         registerNamedObject(obj, name)
#define registerNamedFunction(obj, name)            registerNamedObject(obj, name)
#define registerNamedMesh(obj, name)                registerNamedObject(obj, name)
#define registerNamedMeshModifier(name)             registerNamedObject(obj, name)
#define registerNamedConstraint(obj, name)          registerNamedObject(obj, name)
#define registerNamedUserObject(obj, name)          registerNamedObject(obj, name)
#define registerNamedPreconditioner(obj, name)      registerNamedObject(obj, name)
#define registerNamedIndicator(obj, name)           registerNamedObject(obj, name)
#define registerNamedMarker(obj, name)              registerNamedObject(obj, name)
#define registerNamedTransfer(obj, name)            registerNamedObject(obj, name)
#define registerNamedTimeStepper(obj, name)         registerNamedObject(obj, name)
#define registerNamedTimeIntegrator(obj, name)      registerNamedObject(obj, name)
#define registerNamedPredictor(obj, name)           registerNamedObject(obj, name)
#define registerNamedOutput(obj, name)              registerNamedObject(obj, name)


// Macros for registering deprecated objects
#define registerDeprecatedObject(name, time)        factory.regDeprecated<name>(stringifyName(name), time)
#define registerDeprecatedObjectName(obj, name, time)     factory.regReplaced<obj>(stringifyName(obj), name, time)

/**
 * Typedef for function to build objects
 */
typedef MooseObject * (*buildPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsPtr)();

/**
 * Typedef for registered Object iterator
 */
typedef std::map<std::string, paramsPtr>::iterator registeredMooseObjectIterator;

/**
 * Build an object of type T
 */
template<class T>
MooseObject * buildObject(const std::string & name, InputParameters parameters)
{
  return new T(name, parameters);
}

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
  template<typename T>
  void reg(const std::string & obj_name)
  {
    if (_name_to_build_pointer.find(obj_name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[obj_name] = &buildObject<T>;
      _name_to_params_pointer[obj_name] = &validParams<T>;
    }
    else
      mooseError("Object '" + obj_name + "' already registered.");
  }

  /**
   * Register a deprecated object that expires
   * @param obj_name The name of the object to register
   * @param t_str String contiaining the experiation date for the object
   */
  template<typename T>
  void regDeprecated(const std::string & obj_name, const std::string t_str)
  {
    // Register the name
    reg<T>(obj_name);

    // Store the time
    _deprecated_time[obj_name] = parseTime(t_str);
  }

  /**
   * Register a deprecated object that expires and has a replacement object
   * @param obj_name The name of the object to register (the new object you want people to use)
   * @param name The name of the object that is deprecated
   * @param t_str String contiaining the experiation date for the object
   */
  template<typename T>
  void regReplaced(const std::string & obj_name, const std::string & name, const std::string t_str)
  {
    // Register the name
    regDeprecated<T>(name, t_str);

    // Store the new name
    _deprecated_name[name] = obj_name;
  }

  /**
   * Get valid parameters for the object
   * @param name Name of the object whose parameter we are requesting
   * @return Parameters of the object
   */
  InputParameters getValidParams(const std::string & name);

  /**
   * Build an object (must be registered)
   * @param obj_name Type of the object being constructed
   * @param name Name for the object
   * @param parameters Parameters this object should have
   * @return The created object
   */
  virtual MooseObject *create(const std::string & obj_name, const std::string & name, InputParameters parameters);

  /**
   * Access to registered object iterator (begin)
   */
  registeredMooseObjectIterator registeredObjectsBegin() { return _name_to_params_pointer.begin(); }

  /**
   * Access to registered object iterator (end)
   */
  registeredMooseObjectIterator registeredObjectsEnd() { return _name_to_params_pointer.end(); }

protected:

  /**
   * Parse time string (mm/dd/yyyy HH:MM)
   * @param t_str String with the object experiation date, this must be in the form mm/dd/yyyy HH:MM
   * @return A time_t object with the experation date
   */
  time_t parseTime(std::string);

  /**
   * Show the appropriate message for deprecated objects
   * @param obj_name Name of the deprecated object
   */
  void deprecatedMessage(const std::string obj_name);

  /// Reference to the application
  MooseApp & _app;

  /// Storage for pointers to the object
  std::map<std::string, buildPtr> _name_to_build_pointer;

  /// Storage for pointers to the parameters objects
  std::map<std::string, paramsPtr> _name_to_params_pointer;

  /// Storage for deprecated object experiation dates
  std::map<std::string, time_t> _deprecated_time;

  /// Storage for the deprecated objects that have replacements
  std::map<std::string, std::string> _deprecated_name;

};

#endif /* FACTORY_H */
