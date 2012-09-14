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

#ifndef PROBLEMFACTORY_H
#define PROBLEMFACTORY_H

#include <vector>

#include "InputParameters.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerProblem(name)                       ProblemFactory::instance()->reg<name>(stringifyName(name))

// Forward Declarations
class Problem;
class FEProblem;
class MooseMesh;

/**
 * Typedef for function to build objects
 */
typedef Problem * (*buildProbPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsPtr)();

/**
 * Typedef for registered Object iterator
 */
typedef std::map<std::string, paramsPtr>::iterator registeredProblemsIterator;

/**
 * Build an object of type T
 */
template<class T>
Problem * buildObject(const std::string & name, InputParameters parameters)
{
  return new T(name, parameters);
}

/**
 * Generic ProblemFactory class for build all sorts of objects
 */
class ProblemFactory
{
public:
  /**
   * Get the instance of the factory
   * @return Pointer to the factory instance
   */
  static ProblemFactory *instance();

  virtual ~ProblemFactory();

  /**
   * Release the memory allocated by this factory
   */
  static void release();

  /**
   * Register a new object
   * @param name Name of the object to register
   */
  template<typename T>
  void reg(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildObject<T>;
      _name_to_params_pointer[name] = &validParams<T>;
    }
    else
      mooseError("Problem '" + name + "' already registered.");
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
  virtual Problem *create(const std::string & obj_name, const std::string & name, InputParameters parameters);

  registeredProblemsIterator registeredObjectsBegin() { return _name_to_params_pointer.begin(); }
  registeredProblemsIterator registeredObjectsEnd() { return _name_to_params_pointer.end(); }

protected:
  std::map<std::string, buildProbPtr>  _name_to_build_pointer;
  std::map<std::string, paramsPtr> _name_to_params_pointer;

  static ProblemFactory *_instance;

private:
  // Private constructor for singleton pattern
  ProblemFactory() {}

};

#endif /* PROBLEMFACTORY_H */
