#include "AuxFactory.h"
#include <list> 

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
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
  {
    AuxKernel * aux;
    AuxKernelIterator curr_aux, end_aux;
    unsigned int size;
    
    std::vector<std::list<AuxKernel *>::iterator > dependent_auxs;
    std::vector<AuxKernel *> *aux_ptr;
    std::list<AuxKernel *>::iterator new_aux_iter;

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;

      aux = (*name_to_build_pointer[Aux_name])(name,parameters,var_name,coupled_to,coupled_as);
      
      //
      if (aux->isNodal())
        aux_ptr = &active_NodalAuxKernels[tid];
      else
        aux_ptr = &active_ElementAuxKernels[tid];

      // Copy the active AuxKernels into a list for manipulation
      std::list<AuxKernel *> active_auxs(aux_ptr->begin(), aux_ptr->end());

      // Get a list of all the dependent variables that this AuxKernel will act on to
      // place it in the vector in the appropriate location
      for (std::list<AuxKernel *>::iterator i=active_auxs.begin(); i != active_auxs.end(); ++i)
        for (std::vector<std::string>::iterator j=coupled_to.begin(); j != coupled_to.end(); ++j)
          if ((*i)->varName() == *j) 
            dependent_auxs.push_back(i);

      // Insert the AuxKernel preserving its dependents (last iterator in dependent_auxs)
      if (dependent_auxs.empty())
      {
        active_auxs.push_front(aux);
        new_aux_iter = active_auxs.begin();
      }
      else
        new_aux_iter = active_auxs.insert(++(*dependent_auxs.rbegin()), aux);

      // Now check to see if any of the existing AuxKernels depend on this newly inserted kernel
      dependent_auxs.clear();
      for (std::list<AuxKernel *>::iterator i=active_auxs.begin(); i != new_aux_iter; ++i)
      {
        const std::vector<std::string> & curr_coupled = (*i)->coupledTo();
        for (std::vector<std::string>::const_iterator j=curr_coupled.begin(); j != curr_coupled.end(); ++j)
          if (var_name == *j)
            dependent_auxs.push_back(i);
      }
      
      // Move the dependent items to the point after the insertion of this aux kernel
      ++new_aux_iter;
      for (std::vector<std::list<AuxKernel *>::iterator >::iterator i=dependent_auxs.begin();
           i != dependent_auxs.end(); ++i)
        active_auxs.splice(new_aux_iter, active_auxs, *i);
      
      // DEBUG
      //for (std::list<AuxKernel *>::iterator i = active_auxs.begin(); i != active_auxs.end(); ++i)
      //  std::cout << (*i)->varName() << std::endl;
      //std::cout << "\n" << std::endl;
      // DEBUG

      // Copy the list back into the Auxilary Vector
      aux_ptr->assign(active_auxs.begin(), active_auxs.end());
    }

    return aux;
  }

AuxKernel *
AuxFactory::addBC(std::string Aux_name,
                    std::string name,
                    InputParameters parameters,
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

InputParameters
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

AuxKernelNamesIterator
AuxFactory::registeredAuxKernelsBegin()
{
  // Make sure the _registered_auxkernel_names are up to date
  _registered_auxkernel_names.clear();
  _registered_auxkernel_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, AuxKernelParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_auxkernel_names.push_back(i->first);
  }
  
  return _registered_auxkernel_names.begin();
}

AuxKernelNamesIterator
AuxFactory::registeredAuxKernelsEnd()
{
  return _registered_auxkernel_names.end();
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
