#include "AuxKernel.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>

#ifndef AUXFACTORY_H
#define AUXFACTORY_H

/**
 * Typedef to make things easier.
 */
typedef AuxKernel * (*AuxKernelBuildPtr)(std::string name,
                                         Parameters parameters,
                                         std::string var_name,
                                         std::vector<std::string> coupled_to,
                                         std::vector<std::string> coupled_as);
/**
 * Typedef to make things easier.
 */
typedef Parameters (*AuxKernelParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename AuxType>
AuxKernel * buildAux(std::string name,
                     Parameters parameters,
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
  static AuxFactory * instance()
  {
    static AuxFactory * instance;
    if(!instance)
      instance=new AuxFactory;
    return instance;
  }

  template<typename AuxType> 
  void registerAux(std::string name)
  {
    name_to_build_pointer[name]=&buildAux<AuxType>;
    name_to_params_pointer[name]=&valid_params<AuxType>;
  }

  AuxKernel * add(std::string Aux_name,
                  std::string name,
                  Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                  std::vector<std::string> coupled_as=std::vector<std::string>(0))
  {
    AuxKernel * aux = (*name_to_build_pointer[Aux_name])(name,parameters,var_name,coupled_to,coupled_as);

    active_AuxKernels.push_back(aux);

    return aux;
  }
  
  AuxKernel * addBC(std::string Aux_name,
                    std::string name,
                    Parameters parameters,
                    std::string var_name,
                    unsigned int boundary_id,
                    std::vector<std::string> coupled_to=std::vector<std::string>(0),
                    std::vector<std::string> coupled_as=std::vector<std::string>(0))
  {
    AuxKernel * aux = (*name_to_build_pointer[Aux_name])(name,parameters,var_name,coupled_to,coupled_as);

    active_bcs[boundary_id].push_back(aux);
    

    return aux;
  }

  Parameters getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Aux "<<std::endl<<std::endl;
      error();
    }
    return name_to_params_pointer[name]();
  }

  std::vector<AuxKernel *>::iterator activeAuxKernelsBegin(){ return active_AuxKernels.begin(); };
  std::vector<AuxKernel *>::iterator activeAuxKernelsEnd(){ return active_AuxKernels.end(); };

  std::vector<AuxKernel *>::iterator activeAuxBCsBegin(unsigned int boundary_id){ return active_bcs[boundary_id].begin(); };
  std::vector<AuxKernel *>::iterator activeAuxBCsEnd(unsigned int boundary_id){ return active_bcs[boundary_id].end(); };

private:
  AuxFactory(){}
  virtual ~AuxFactory(){}

  std::map<std::string, AuxKernelBuildPtr> name_to_build_pointer;
  std::map<std::string, AuxKernelParamsPtr> name_to_params_pointer;

  std::vector<AuxKernel *> active_AuxKernels;

  std::map<unsigned int, std::vector<AuxKernel *> > active_bcs;
};

#endif //AUXFACTORY_H
