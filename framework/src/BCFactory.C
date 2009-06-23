#include "BCFactory.h"

BCFactory *
BCFactory::instance()
  {
    static BCFactory * instance;
    if(!instance)
      instance=new BCFactory;
    return instance;
  }

 void
 BCFactory::add(std::string bc_name,
           std::string name,
           Parameters parameters,
           std::string var_name,
           unsigned int boundary_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
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

Parameters
BCFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered BC "<<std::endl<<std::endl;
      error();
    }

    return name_to_params_pointer[name]();
  }

std::vector<BoundaryCondition *>::iterator
BCFactory::activeBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].begin();
}

std::vector<BoundaryCondition *>::iterator
BCFactory::activeBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].end();
}


std::vector<BoundaryCondition *>::iterator
BCFactory::activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_nodal_bcs[tid][boundary_id].begin();
}

std::vector<BoundaryCondition *>::iterator
BCFactory::activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_nodal_bcs[tid][boundary_id].end();
}

BCFactory::BCFactory()
  {
    active_bcs.resize(libMesh::n_threads());
    active_nodal_bcs.resize(libMesh::n_threads());
  }
 
