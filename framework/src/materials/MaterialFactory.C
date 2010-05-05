#include "MaterialFactory.h"
#include "MooseSystem.h"

//libMesh Includes
#include "equation_systems.h"

#include <iostream>

MaterialFactory *
MaterialFactory::instance()
     {
    static MaterialFactory * instance;
    if(!instance)
      instance=new MaterialFactory;
    return instance;
  }

void
  MaterialFactory::add(std::string mat_name,
                       std::string name,
                       MooseSystem & moose_system,
                       InputParameters parameters)
  {
    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      Moose::current_thread_id = tid;
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");

      // TODO: Remove this hack when Material no longer inherits from Kernel!
      parameters.set<std::string>("variable") = Kernel::_es->get_system(0).variable_name(0);

      for (unsigned int i=0; i<blocks.size(); ++i)
        active_materials[tid][blocks[i]] = (*name_to_build_pointer[mat_name])(name, moose_system, parameters);
    }
  }

InputParameters
MaterialFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not registered Material "<<std::endl<<std::endl;
      mooseError("");
    }
    return name_to_params_pointer[name]();
  }

Material *
MaterialFactory::getMaterial(THREAD_ID tid, unsigned int block_id)
  {
    MaterialIterator mat_iter = active_materials[tid].find(block_id);
    if (mat_iter == active_materials[tid].end()) 
    {
      std::cerr << "Active Material Missing\n";
      mooseError("");
    }
    return mat_iter->second;
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


MaterialNamesIterator
MaterialFactory::registeredMaterialsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_material_names.clear();
  _registered_material_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, MaterialParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_material_names.push_back(i->first);
  }
  
  return _registered_material_names.begin();
}

MaterialNamesIterator
MaterialFactory::registeredMaterialsEnd()
{
  return _registered_material_names.end();
}


MaterialIterator
MaterialFactory::activeMaterialsBegin(THREAD_ID tid)
{
  return active_materials[tid].begin();
}

MaterialIterator
MaterialFactory::activeMaterialsEnd(THREAD_ID tid)
{
  return active_materials[tid].end();
}

MaterialFactory::MaterialFactory()
{
  active_materials.resize(libMesh::n_threads());
}

