#include "MaterialFactory.h"
#include <iostream>

  void
  MaterialFactory::add(std::string mat_name,
           std::string name,
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
  {
    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;

      active_materials[tid][block_id] = (*name_to_build_pointer[mat_name])(name,parameters,block_id,coupled_to,coupled_as);
    }
  }

Parameters
MaterialFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not registered Material "<<std::endl<<std::endl;
      error();
    }
    return name_to_params_pointer[name]();
  }

void MaterialFactory::updateMaterialDataState()
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  { 
    std::map<int, Material *>::iterator it = active_materials[tid].begin();
    std::map<int, Material *>::iterator it_end = active_materials[tid].end();
    
    for(;it!=it_end;++it) 
      it->second->updateDataState();
  }
}

MaterialFactory::MaterialFactory()
  {
    active_materials.resize(libMesh::n_threads());
  }

