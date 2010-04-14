#include "StabilizerFactory.h"

StabilizerFactory *
StabilizerFactory::instance()
{
  static StabilizerFactory * instance;
  if(!instance)
    instance=new StabilizerFactory;
    
  return instance;
}

Stabilizer *
StabilizerFactory::add(std::string stabilizer_name,
                       std::string name,
                       MooseSystem & moose_system,
                       InputParameters parameters)
{
  Stabilizer * stabilizer;
    
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    stabilizer = (*name_to_build_pointer[stabilizer_name])(name, moose_system, parameters);

    if (parameters.have_parameter<unsigned int>("block_id"))
      block_stabilizers[tid][parameters.get<unsigned int>("block_id")][stabilizer->variable()] = stabilizer;
    else
      active_stabilizers[tid][stabilizer->variable()] = stabilizer;

    _is_stabilized[stabilizer->variable()] = true;
  }

  return stabilizer;
}

InputParameters
StabilizerFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Stabilizer "<<std::endl<<std::endl;
    mooseError("");
  }
  return name_to_params_pointer[name]();
}

bool
StabilizerFactory::isStabilized(unsigned int var_num)
{
  return _is_stabilized[var_num];
}  

StabilizerIterator
StabilizerFactory::activeStabilizersBegin(THREAD_ID tid)
{
  return active_stabilizers[tid].begin();
}

StabilizerIterator
StabilizerFactory::activeStabilizersEnd(THREAD_ID tid)
{
  return active_stabilizers[tid].end();
}


StabilizerIterator
StabilizerFactory::blockStabilizersBegin(THREAD_ID tid, unsigned int block_id)
{
  return block_stabilizers[tid][block_id].begin();
}

StabilizerIterator
StabilizerFactory::blockStabilizersEnd(THREAD_ID tid, unsigned int block_id)
{
  return block_stabilizers[tid][block_id].end();
}

StabilizerNamesIterator
StabilizerFactory::registeredStabilizersBegin()
{
  // Make sure the _registered_stabilizer_names are up to date
  _registered_stabilizer_names.clear();
  _registered_stabilizer_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, stabilizerParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_stabilizer_names.push_back(i->first);
  }
  
  return _registered_stabilizer_names.begin();
}

StabilizerNamesIterator
StabilizerFactory::registeredStabilizersEnd()
{
  return _registered_stabilizer_names.end();
}

StabilizerFactory::StabilizerFactory()
{
  active_stabilizers.resize(libMesh::n_threads());
  block_stabilizers.resize(libMesh::n_threads());
}
  
StabilizerFactory:: ~StabilizerFactory() 
{
  {
    std::map<std::string, stabilizerBuildPtr>:: iterator i;
    for(i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, stabilizerParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
     
  {
        
    std::vector<std::map<unsigned int, Stabilizer *> >::iterator i;
    for (i=active_stabilizers.begin(); i!=active_stabilizers.end(); ++i)
    { 

      StabilizerIterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        delete j->second;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::map<unsigned int, Stabilizer *> > >::iterator i;
    for (i=block_stabilizers.begin(); i!=block_stabilizers.end(); ++i)
    {
          
      std::map<unsigned int, std::map<unsigned int, Stabilizer *> >::iterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        StabilizerIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete k->second;
        }
      }
    }
  }
}

