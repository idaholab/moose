#ifndef AUXFACTORY_H
#define AUXFACTORY_H

#include "AuxKernel.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef AuxKernel * (*AuxKernelBuildPtr)(std::string name,
                                         MooseSystem & moose_system,
                                         InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator AuxKernelNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*AuxKernelParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename AuxType>
AuxKernel * buildAux(std::string name,
                     MooseSystem & moose_system,
                     InputParameters parameters)
{
  return new AuxType(name, moose_system, parameters);
}

/**
 * Responsible for building AuxKernels on demand and storing them for retrieval
 *
 * Note that this class is also responsible for AuxKernels on boundaries.
 */
class AuxFactory
{
public:
  static AuxFactory * instance();
  
  template<typename AuxType> 
  void registerAux(std::string name)
  {
    _name_to_build_pointer[name]=&buildAux<AuxType>;
    _name_to_params_pointer[name]=&validParams<AuxType>;
  }

  AuxKernel *create(std::string aux_name,
                     std::string name,
                     MooseSystem & moose_system,
                     InputParameters parameters)
  {
    return (*_name_to_build_pointer[aux_name])(name, moose_system, parameters);
  }

  InputParameters getValidParams(std::string name);

  AuxKernelNamesIterator registeredAuxKernelsBegin();
  AuxKernelNamesIterator registeredAuxKernelsEnd();

private:
  AuxFactory();

  virtual ~AuxFactory();

  std::map<std::string, AuxKernelBuildPtr> _name_to_build_pointer;
  std::map<std::string, AuxKernelParamsPtr> _name_to_params_pointer;
  
  std::vector<std::string> _registered_auxkernel_names;
};

#endif //AUXFACTORY_H
