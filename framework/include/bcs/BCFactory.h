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

#ifndef BCFACTORY_H
#define BCFACTORY_H

#include "BoundaryCondition.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef BoundaryCondition * (*BCBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator BCNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*BCParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename BCType>
BoundaryCondition * buildBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
{
  return new BCType(name, moose_system, parameters);
}

/**
 * Responsible for building BCs on demand and storing them for retrieval
 */
class BCFactory
{
public:
  static BCFactory * instance();
  
  template<typename BCType> 
  void registerBC(std::string name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildBC<BCType>;
      _name_to_params_pointer[name] = &validParams<BCType>;
    }
    else
      mooseError("BoundaryCondition '" + name + "' already registered.");
  }

  BoundaryCondition *create(std::string bc_name,
                            std::string name,
                            MooseSystem & moose_system,
                            InputParameters parameters)
  {
    return (*_name_to_build_pointer[bc_name])(name, moose_system, parameters);
  }

  BCNamesIterator registeredBCsBegin();
  BCNamesIterator registeredBCsEnd();

  InputParameters getValidParams(std::string name);
  
private:
  BCFactory();

  virtual ~BCFactory();

  std::map<std::string, BCBuildPtr> _name_to_build_pointer;
  std::map<std::string, BCParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_bc_names;

};

#endif //BCFACTORY_H
