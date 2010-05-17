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
typedef Stabilizer * (*stabilizerBuildPtr)(std::string name,
                                           MooseSystem & moose_system,
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
Stabilizer * buildStabilizer(std::string name,
                             MooseSystem & moose_system,
                             InputParameters parameters)
{
  return new StabilizerType(name, moose_system, parameters);
}

/**
 * Responsible for building Stabilizers on demand and storing them for retrieval
 */
class StabilizerFactory
{
public:
  static StabilizerFactory * instance();

  template<typename StabilizerType> 
  void registerStabilizer(std::string name)
  {
    _name_to_build_pointer[name]=&buildStabilizer<StabilizerType>;
    _name_to_params_pointer[name]=&validParams<StabilizerType>;
  }

  Stabilizer * create(std::string stabilizer_name,
                      std::string name,
                      MooseSystem & moose_system,
                      InputParameters parameters)
  {
    return (*_name_to_build_pointer[stabilizer_name])(name, moose_system, parameters);
  }

  StabilizerNamesIterator registeredStabilizersBegin();
  StabilizerNamesIterator registeredStabilizersEnd();

  InputParameters getValidParams(std::string name);

private:
  StabilizerFactory();

  virtual ~StabilizerFactory();
  
  std::map<std::string, stabilizerBuildPtr> _name_to_build_pointer;
  std::map<std::string, stabilizerParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_stabilizer_names;
};

#endif //STABILIZERFACTORY_H
