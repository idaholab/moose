#include "MaterialFactory.h"
#include <iostream>

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
