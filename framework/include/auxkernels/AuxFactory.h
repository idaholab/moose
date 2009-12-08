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
                                         InputParameters parameters,
                                         std::string var_name,
                                         std::vector<std::string> coupled_to,
                                         std::vector<std::string> coupled_as);

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
                     InputParameters parameters,
                     std::string var_name,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as)
{
  return new AuxType(name, parameters, var_name, coupled_to, coupled_as);
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
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                  std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
  AuxKernel * addBC(std::string Aux_name,
                    std::string name,
                    InputParameters parameters,
                    std::string var_name,
                    unsigned int boundary_id,
                    std::vector<std::string> coupled_to=std::vector<std::string>(0),
                    std::vector<std::string> coupled_as=std::vector<std::string>(0));

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
