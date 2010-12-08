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

#ifndef STABILIZERFACTORY_H
#define STABILIZERFACTORY_H

#include "Stabilizer.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef Stabilizer * (*stabilizerBuildPtr)(const std::string & name,
                                           InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator StabilizerNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*stabilizerParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename StabilizerType>
Stabilizer * buildStabilizer(const std::string & name,
                             InputParameters parameters)
{
  return new StabilizerType(name, parameters);
}

/**
 * Responsible for building Stabilizers on demand and storing them for retrieval
 */
class StabilizerFactory
{
public:
  static StabilizerFactory * instance();

  template<typename StabilizerType> 
  void registerStabilizer(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildStabilizer<StabilizerType>;
      _name_to_params_pointer[name] = &validParams<StabilizerType>;
    }
    else
      mooseError("Stabilizer '" + name + "' already registered.");
  }

  Stabilizer * create(std::string stabilizer_name,
                      const std::string & name,
                      InputParameters parameters)
  {
    return (*_name_to_build_pointer[stabilizer_name])(name, parameters);
  }

  StabilizerNamesIterator registeredStabilizersBegin();
  StabilizerNamesIterator registeredStabilizersEnd();

  InputParameters getValidParams(const std::string & name);

private:
  StabilizerFactory();

  virtual ~StabilizerFactory();
  
  std::map<std::string, stabilizerBuildPtr> _name_to_build_pointer;
  std::map<std::string, stabilizerParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_stabilizer_names;
};

#endif //STABILIZERFACTORY_H
