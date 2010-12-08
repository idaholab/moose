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

#ifndef DAMPERFACTORY_H
#define DAMPERFACTORY_H

#include "Damper.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>

// forward declarations
class MooseSystem;

/**
 * Typedef to make things easier.
 */
typedef Damper * (*damperBuildPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator DamperNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*damperParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename DamperType>
Damper * buildDamper(const std::string & name, InputParameters parameters)
{
  return new DamperType(name, parameters);
}

/**
 * Responsible for building Dampers on demand and storing them for retrieval
 */
class DamperFactory
{
public:
  static DamperFactory * instance();

  template<typename DamperType> 
  void registerDamper(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildDamper<DamperType>;
      _name_to_params_pointer[name] = &validParams<DamperType>;
    }
    else
      mooseError("Damper '" + name + "' already registered.");
  }

  Damper *create(std::string damper_name, const std::string & name, InputParameters parameters)
  {
    return (*_name_to_build_pointer[damper_name])(name, parameters);
  }

  DamperNamesIterator registeredDampersBegin();
  DamperNamesIterator registeredDampersEnd();

  InputParameters getValidParams(const std::string & name);
  
private:
  DamperFactory();

  virtual ~DamperFactory();
  
  std::map<std::string, damperBuildPtr> _name_to_build_pointer;
  std::map<std::string, damperParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_damper_names;
};

#endif //DAMPERFACTORY_H
