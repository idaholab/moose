#include "BoundaryCondition.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>

#ifndef BCFACTORY_H
#define BCFACTORY_H

/**
 * Typedef to make things easier.
 */
typedef BoundaryCondition * (*BCBuildPtr)(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

/**
 * Typedef to make things easier.
 */
typedef Parameters (*BCParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename BCType>
BoundaryCondition * buildBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
{
  return new BCType(name, parameters, var_name, boundary_id, coupled_to, coupled_as);
}

/**
 * Responsible for building BCs on demand and storing them for retrieval
 */
class BCFactory
{
public:
  static BCFactory * instance()
  {
    static BCFactory * instance;
    if(!instance)
      instance=new BCFactory;
    return instance;
  }

  template<typename BCType> 
  void registerBC(std::string name)
  {
    name_to_build_pointer[name]=&buildBC<BCType>;
    name_to_params_pointer[name]=&valid_params<BCType>;
  }

  void add(std::string bc_name,
           std::string name,
           Parameters parameters,
           std::string var_name,
           unsigned int boundary_id,
           std::vector<std::string> coupled_to=std::vector<std::string>(0),
           std::vector<std::string> coupled_as=std::vector<std::string>(0))
  {
    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;

      BoundaryCondition * bc = (*name_to_build_pointer[bc_name])(name,parameters,var_name,boundary_id, coupled_to, coupled_as);

      if(bc->isIntegrated())
        active_bcs[tid][boundary_id].push_back(bc);
      else
        active_nodal_bcs[tid][boundary_id].push_back(bc);
    }
  }

  Parameters getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered BC "<<std::endl<<std::endl;
      error();
    }

    return name_to_params_pointer[name]();
  }

  std::vector<BoundaryCondition *>::iterator activeBCsBegin(THREAD_ID tid, unsigned int boundary_id){ return active_bcs[tid][boundary_id].begin(); };
  std::vector<BoundaryCondition *>::iterator activeBCsEnd(THREAD_ID tid, unsigned int boundary_id){ return active_bcs[tid][boundary_id].end(); };

  std::vector<BoundaryCondition *>::iterator activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id){ return active_nodal_bcs[tid][boundary_id].begin(); };
  std::vector<BoundaryCondition *>::iterator activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id){ return active_nodal_bcs[tid][boundary_id].end(); };

private:
  BCFactory()
  {
    active_bcs.resize(libMesh::n_threads());
    active_nodal_bcs.resize(libMesh::n_threads());
  }
  virtual ~BCFactory(){}

  std::map<std::string, BCBuildPtr> name_to_build_pointer;
  std::map<std::string, BCParamsPtr> name_to_params_pointer;

  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > active_bcs;
  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > active_nodal_bcs;
};

#endif //BCFACTORY_H
