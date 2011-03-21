#ifndef FACTORY_H_
#define FACTORY_H_

#include <vector>

#include "Moose.h"
#include "Object.h"
#include "InputParameters.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerObject(name)                      Factory::instance()->reg<name>(stringifyName(name))

#define registerKernel(name)                      Factory::instance()->reg<name>(stringifyName(name))
#define registerBoundaryCondition(name)           Factory::instance()->reg<name>(stringifyName(name))


/**
 * Typedef for function to build objects
 */
typedef Object * (*buildPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsPtr)();


/**
 * Build an object of type T
 */
template<class T>
Object * buildObject(const std::string & name, InputParameters parameters)
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

  virtual Object *create(const std::string & obj_name, const std::string & name, InputParameters parameters);

protected:
  std::map<std::string, buildPtr>  _name_to_build_pointer;
  std::map<std::string, paramsPtr> _name_to_params_pointer;
};

#endif /* FACTORY_H_ */
