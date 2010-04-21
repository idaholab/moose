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
                MooseSystem & moose_system,
                InputParameters parameters)
  {
    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;
      std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

      for (unsigned int i=0; i<boundaries.size(); ++i)
      {
        parameters.set<unsigned int>("_boundary_id") = boundaries[i];
        BoundaryCondition * bc = (*name_to_build_pointer[bc_name])(name, moose_system, parameters);

        if(bc->isIntegrated())
          active_bcs[tid][boundaries[i]].push_back(bc);
        else
          active_nodal_bcs[tid][boundaries[i]].push_back(bc);
      }
    }
  }

InputParameters
BCFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered BC "<<std::endl<<std::endl;
      mooseError("");
    }

    return name_to_params_pointer[name]();
  }

BCIterator
BCFactory::activeBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].begin();
}

BCIterator
BCFactory::activeBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].end();
}


BCIterator
BCFactory::activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_nodal_bcs[tid][boundary_id].begin();
}

BCIterator
BCFactory::activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_nodal_bcs[tid][boundary_id].end();
}

BCNamesIterator
BCFactory::registeredBCsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_bc_names.clear();
  _registered_bc_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, BCParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_bc_names.push_back(i->first);
  }
  
  return _registered_bc_names.begin();
}

BCNamesIterator
BCFactory::registeredBCsEnd()
{
  return _registered_bc_names.end();
}

void
BCFactory::activeBoundaries(std::set<subdomain_id_type> & set_buffer) const
{
  std::map<unsigned int, std::vector<BoundaryCondition *> >::const_iterator curr, end;
  end = active_bcs[0].end();

  try 
  {
    for (curr = active_bcs[0].begin(); curr != end; ++curr)
    {
      set_buffer.insert(subdomain_id_type(curr->first));
    }
  }
  catch (std::exception &e)
  {
    mooseError("Invalid block specified in input file");
  }
}


BCFactory::BCFactory()
{
  active_bcs.resize(libMesh::n_threads());
  active_nodal_bcs.resize(libMesh::n_threads());
}

BCFactory::~BCFactory()
{
  {
    std::map<std::string, BCBuildPtr>::iterator i;
    for (i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, BCParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }

  }

  {
    std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > >::iterator i;
    for(i=active_bcs.begin(); i!=active_bcs.end(); ++i)
    {
      std::map<unsigned int, std::vector<BoundaryCondition *> >::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        BCIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > >::iterator i;
    for(i=active_nodal_bcs.begin(); i!=active_nodal_bcs.end(); ++i)
    {
      std::map<unsigned int, std::vector<BoundaryCondition *> >::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        BCIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }

}


 
