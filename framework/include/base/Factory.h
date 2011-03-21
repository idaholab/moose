#ifndef FACTORY_H
#define FACTORY_H

#include <vector>

#include "Moose.h"
#include "MooseObject.h"
#include "InputParameters.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerObject(name)                      Factory::instance()->reg<name>(stringifyName(name))
#define registerNamedObject(obj, name)            Factory::instance()->reg<obj>(name)

// for backward compatibility
#define registerKernel(name)                      registerObject(name)
#define registerBoundaryCondition(name)           registerObject(name)
#define registerAux(name)                         registerObject(name)
#define registerMaterial(name)                    registerObject(name)
#define registerPostprocessor(name)               registerObject(name)
#define registerInitialCondition(name)            registerObject(name)
#define registerDumper(name)                      registerObject(name)
#define registerDiracKernel(name)                 registerObject(name)
#define registerExecutioner(name)                 registerObject(name)

#define registerNamedKernel(obj, name)                 registerNamedObject(obj, name)
#define registerNamedBoundaryCondition(obj, name)      registerNamedObject(obj, name)
#define registerNamedAux(obj, name)                    registerNamedObject(obj, name)
#define registerNamedMaterial(obj, name)               registerNamedObject(obj, name)
#define registerNamedPostprocessor(obj, name)          registerNamedObject(obj, name)
#define registerNamedInitialCondition(obj, name)       registerNamedObject(obj, name)
#define registerNamedDumper(obj, name)                 registerNamedObject(obj, name)
#define registerNamedDiracKernel(obj, name)            registerNamedObject(obj, name)
#define registerNamedExecutioner(obj, name)            registerNamedObject(obj, name)


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
  static Factory *instance();

  template<typename T>
  void reg(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildObject<T>;
      _name_to_params_pointer[name] = &validParams<T>;
    }
    else
      mooseError("Object '" + name + "' already registered.");
  }

  InputParameters getValidParams(const std::string & name);

  virtual MooseObject *create(const std::string & obj_name, const std::string & name, InputParameters parameters);

  registeredMooseObjectIterator registeredObjectsBegin() { return _name_to_params_pointer.begin(); }
  registeredMooseObjectIterator registeredObjectsEnd() { return _name_to_params_pointer.end(); }
  
protected:
  std::map<std::string, buildPtr>  _name_to_build_pointer;
  std::map<std::string, paramsPtr> _name_to_params_pointer;

private:
  // Private constructor for singleton pattern
  Factory() {}
  
};

#endif /* FACTORY_H */
