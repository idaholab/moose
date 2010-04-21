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
typedef std::vector<AuxKernel *>::iterator AuxKernelIterator;

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
    name_to_build_pointer[name]=&buildAux<AuxType>;
    name_to_params_pointer[name]=&validParams<AuxType>;
  }

  AuxKernel * add(std::string Aux_name,
                  std::string name,
                  MooseSystem & moose_system,
                  InputParameters parameters);
  
  AuxKernel * addBC(std::string Aux_name,
                    std::string name,
                    MooseSystem & moose_system,
                    InputParameters parameters);

  InputParameters getValidParams(std::string name);
  
  std::vector<AuxKernel *>::iterator activeNodalAuxKernelsBegin(THREAD_ID tid);
  std::vector<AuxKernel *>::iterator activeNodalAuxKernelsEnd(THREAD_ID tid);

  std::vector<AuxKernel *>::iterator activeElementAuxKernelsBegin(THREAD_ID tid);
  std::vector<AuxKernel *>::iterator activeElementAuxKernelsEnd(THREAD_ID tid);

  std::vector<AuxKernel *>::iterator activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  std::vector<AuxKernel *>::iterator activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  AuxKernelNamesIterator registeredAuxKernelsBegin();
  AuxKernelNamesIterator registeredAuxKernelsEnd();

private:
  AuxFactory();

  virtual ~AuxFactory();

  std::map<std::string, AuxKernelBuildPtr> name_to_build_pointer;
  std::map<std::string, AuxKernelParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_auxkernel_names;
  std::vector<std::vector<AuxKernel *> > active_NodalAuxKernels;
  std::vector<std::vector<AuxKernel *> > active_ElementAuxKernels;

  std::vector<std::map<unsigned int, std::vector<AuxKernel *> > > active_bcs;
  
};

#endif //AUXFACTORY_H
