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

#ifndef APPFACTORY_H
#define APPFACTORY_H

#include <vector>

#include "MooseApp.h"
#include "InputParameters.h"

/**
 * Macros
 */
#define registerApp(name)                        AppFactory::instance().reg<name>(#name)

/**
 * Typedef for function to build objects
 */
typedef MooseApp * (*appBuildPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsPtr)();

/**
 * Typedef for registered Object iterator
 */
typedef std::map<std::string, paramsPtr>::iterator registeredMooseAppIterator;

/**
 * Build an object of type T
 */
template<class T>
MooseApp * buildApp(const std::string & name, InputParameters parameters)
{
  return new T(name, parameters);
}


/**
 * Generic AppFactory class for build all sorts of objects
 */
class AppFactory
{
public:
  /**
   * Get the instance of the AppFactory
   * @return Pointer to the AppFactory instance
   */
  static AppFactory & instance();

  virtual ~AppFactory();

  /**
   * Helper function for creating a MooseApp from command-line arguments.
   */
  static MooseApp * createApp(std::string app_type, int argc, char ** argv);

  /**
   * Register a new object
   * @param name Name of the object to register
   */
  template<typename T>
  void reg(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildApp<T>;
      _name_to_params_pointer[name] = &validParams<T>;
    }
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
  virtual MooseApp *create(const std::string & obj_name, const std::string & name, InputParameters parameters);

  registeredMooseAppIterator registeredObjectsBegin() { return _name_to_params_pointer.begin(); }
  registeredMooseAppIterator registeredObjectsEnd() { return _name_to_params_pointer.end(); }

protected:
  std::map<std::string, appBuildPtr>  _name_to_build_pointer;
  std::map<std::string, paramsPtr> _name_to_params_pointer;

  static AppFactory _instance;

private:
  // Private constructor for singleton pattern
  AppFactory() {}

};

#endif /* APPFACTORY_H */
