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
typedef Postprocessor * (*postprocessorBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters);

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
Postprocessor * buildPostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
{
  return new PostprocessorType(name, moose_system, parameters);
}

/**
 * Responsible for building Postprocessors on demand and storing them for retrieval
 */
class PostprocessorFactory
{
public:
  static PostprocessorFactory * instance();

  template<typename PostprocessorType> 
  void registerPostprocessor(std::string name)
  {
    _name_to_build_pointer[name]=&buildPostprocessor<PostprocessorType>;
    _name_to_params_pointer[name]=&validParams<PostprocessorType>;
  }

  Postprocessor *create(std::string postprocessor_name, std::string name, MooseSystem & moose_system, InputParameters parameters)
  {
    return (*_name_to_build_pointer[postprocessor_name])(name, moose_system, parameters);
  }

  PostprocessorNamesIterator registeredPostprocessorsBegin();
  PostprocessorNamesIterator registeredPostprocessorsEnd();

  InputParameters getValidParams(std::string name);
  
private:
  PostprocessorFactory();

  virtual ~PostprocessorFactory();
  
  std::map<std::string, postprocessorBuildPtr> _name_to_build_pointer;
  std::map<std::string, postprocessorParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_postprocessor_names;
};

#endif //POSTPROCESSORFACTORY_H
