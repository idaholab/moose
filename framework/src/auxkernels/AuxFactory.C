#include "AuxFactory.h"
 

AuxFactory *
AuxFactory::instance()
  {
    static AuxFactory * instance;
    if(!instance)
      instance=new AuxFactory;
    return instance;
  }

AuxKernel *
AuxFactory::add(std::string Aux_name,
                  std::string name,
                  Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
  {
    AuxKernel * aux;
    
    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;

      aux = (*name_to_build_pointer[Aux_name])(name,parameters,var_name,coupled_to,coupled_as);

      if(aux->isNodal())
        active_NodalAuxKernels[tid].push_back(aux);
      else
        active_ElementAuxKernels[tid].push_back(aux);
    }

    return aux;
  }

AuxKernel *
AuxFactory::addBC(std::string Aux_name,
                    std::string name,
                    Parameters parameters,
                    std::string var_name,
                    unsigned int boundary_id,
                    std::vector<std::string> coupled_to,
                    std::vector<std::string> coupled_as)
  {
    AuxKernel * aux;
    
    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;
      
      aux = (*name_to_build_pointer[Aux_name])(name,parameters,var_name,coupled_to,coupled_as);

      active_bcs[tid][boundary_id].push_back(aux);
    }

    return aux;
  }

Parameters
 AuxFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {

      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Aux "<<std::endl<<std::endl;
      mooseError("");

    }
    return name_to_params_pointer[name]();
  }

std::vector<AuxKernel *>::iterator
AuxFactory::activeNodalAuxKernelsBegin(THREAD_ID tid)
{
  return active_NodalAuxKernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeNodalAuxKernelsEnd(THREAD_ID tid)
{
  return active_NodalAuxKernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeElementAuxKernelsBegin(THREAD_ID tid)
{
  return active_ElementAuxKernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeElementAuxKernelsEnd(THREAD_ID tid)
{
  return active_ElementAuxKernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].begin();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].end();
}
  
AuxFactory::AuxFactory()
  {
    active_NodalAuxKernels.resize(libMesh::n_threads());
    active_ElementAuxKernels.resize(libMesh::n_threads());
    active_bcs.resize(libMesh::n_threads());
  }

AuxFactory:: ~AuxFactory()
{
  {
    std::map<std::string, AuxKernelBuildPtr>::iterator i;
    for(i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, AuxKernelParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::vector<std::vector<AuxKernel *> >::iterator i;
    for(i=active_NodalAuxKernels.begin(); i!=active_NodalAuxKernels.end(); ++i)
    {
      std::vector<AuxKernel *>::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }
  
  {
    std::vector<std::vector<AuxKernel *> >::iterator i;
    for(i=active_ElementAuxKernels.begin(); i!=active_ElementAuxKernels.end(); ++i)
    {
      std::vector<AuxKernel *>::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<AuxKernel *> > >::iterator i;
    for(i=active_bcs.begin(); i!=active_bcs.end(); ++i)
    {
      std::map<unsigned int, std::vector<AuxKernel *> >::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        std::vector<AuxKernel *>::iterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }

}
