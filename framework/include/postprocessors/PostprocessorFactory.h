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

#ifndef POSTPROCESSORFACTORY_H
#define POSTPROCESSORFACTORY_H

#include "Postprocessor.h"

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
typedef Postprocessor * (*postprocessorBuildPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator PostprocessorNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*postprocessorParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename PostprocessorType>
Postprocessor * buildPostprocessor(const std::string & name, InputParameters parameters)
{
  return new PostprocessorType(name, parameters);
}

/**
 * Responsible for building Postprocessors on demand and storing them for retrieval
 */
class PostprocessorFactory
{
public:
  static PostprocessorFactory * instance();

  template<typename PostprocessorType> 
  void registerPostprocessor(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildPostprocessor<PostprocessorType>;
      _name_to_params_pointer[name] = &validParams<PostprocessorType>;
    }
    else
      mooseError("Postprocessor '" + name + "' already registered.");
  }

  Postprocessor *create(std::string postprocessor_name, const std::string & name, InputParameters parameters)
  {
    return (*_name_to_build_pointer[postprocessor_name])(name, parameters);
  }

  PostprocessorNamesIterator registeredPostprocessorsBegin();
  PostprocessorNamesIterator registeredPostprocessorsEnd();

  InputParameters getValidParams(const std::string & name);
  
private:
  PostprocessorFactory();

  virtual ~PostprocessorFactory();
  
  std::map<std::string, postprocessorBuildPtr> _name_to_build_pointer;
  std::map<std::string, postprocessorParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_postprocessor_names;
};

#endif //POSTPROCESSORFACTORY_H
