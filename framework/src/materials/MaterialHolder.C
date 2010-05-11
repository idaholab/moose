#include "MaterialHolder.h"

MaterialHolder::MaterialHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  active_materials.resize(libMesh::n_threads());
}

MaterialHolder::~MaterialHolder()
{
  {
    std::vector<std::map<int, Material *> >::iterator i;
    for (i = active_materials.begin(); i != active_materials.end(); ++i)
    {
      std::map<int, Material *>::iterator j;
      for (j = i->begin(); j != i->end(); ++j)
        delete j->second;
    }
  }
}

Material *
MaterialHolder::getMaterial(THREAD_ID tid, unsigned int block_id)
  {
    MaterialIterator mat_iter = active_materials[tid].find(block_id);
    if (mat_iter == active_materials[tid].end())
    {
      std::cerr << "Active Material Missing\n";
      mooseError("");
    }
    return mat_iter->second;
  }

void MaterialHolder::updateMaterialDataState()
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    std::map<int, Material *>::iterator it = active_materials[tid].begin();
    std::map<int, Material *>::iterator it_end = active_materials[tid].end();

    for(;it!=it_end;++it)
      it->second->updateDataState();
  }
}

MaterialIterator
MaterialHolder::activeMaterialsBegin(THREAD_ID tid)
{
  return active_materials[tid].begin();
}

MaterialIterator
MaterialHolder::activeMaterialsEnd(THREAD_ID tid)
{
  return active_materials[tid].end();
}
