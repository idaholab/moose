#include "MaterialHolder.h"

MaterialHolder::MaterialHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_materials.resize(libMesh::n_threads());
  _active_boundary_materials.resize(libMesh::n_threads());
}

MaterialHolder::~MaterialHolder()
{
  for (std::vector<std::map<int, Material *> >::iterator i = _active_materials.begin(); i != _active_materials.end(); ++i)
  {
    std::map<int, Material *>::iterator j;
    for (j = i->begin(); j != i->end(); ++j)
      delete j->second;
  }

  for (std::vector<std::map<int, Material *> >::iterator i = _active_boundary_materials.begin(); i != _active_boundary_materials.end(); ++i)
  {
    std::map<int, Material *>::iterator j;
    for (j = i->begin(); j != i->end(); ++j)
      delete j->second;
  }
}

Material *
MaterialHolder::getMaterial(THREAD_ID tid, unsigned int block_id)
  {
    MaterialIterator mat_iter = _active_materials[tid].find(block_id);
    if (mat_iter == _active_materials[tid].end())
    {
      std::cerr << "Active Material Missing\n";
      mooseError("");
    }
    return mat_iter->second;
  }

Material *
MaterialHolder::getBoundaryMaterial(THREAD_ID tid, unsigned int boundary_id)
  {
    MaterialIterator mat_iter = _active_boundary_materials[tid].find(boundary_id);
    if (mat_iter == _active_boundary_materials[tid].end())
    {
      std::cerr << "Active Boundary Material Missing\n";
      mooseError("");
    }
    return mat_iter->second;
  }

void MaterialHolder::updateMaterialDataState()
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    std::map<int, Material *>::iterator it = _active_materials[tid].begin();
    std::map<int, Material *>::iterator it_end = _active_materials[tid].end();

    for(;it!=it_end;++it)
    {
      it->second->updateDataState();
      it->second->timeStepSetup();
    }

    it = _active_boundary_materials[tid].begin();
    it_end = _active_boundary_materials[tid].end();

    for(;it!=it_end;++it)
    {
      it->second->updateDataState();
      it->second->timeStepSetup();
    }
  }
}

MaterialIterator
MaterialHolder::activeMaterialsBegin(THREAD_ID tid)
{
  return _active_materials[tid].begin();
}

MaterialIterator
MaterialHolder::activeMaterialsEnd(THREAD_ID tid)
{
  return _active_materials[tid].end();
}

MaterialIterator
MaterialHolder::activeBoundaryMaterialsBegin(THREAD_ID tid)
{
  return _active_boundary_materials[tid].begin();
}

MaterialIterator
MaterialHolder::activeBoundaryMaterialsEnd(THREAD_ID tid)
{
  return _active_boundary_materials[tid].end();
}
