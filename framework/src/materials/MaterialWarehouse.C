#include "MaterialWarehouse.h"

MaterialWarehouse::MaterialWarehouse(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_materials.resize(libMesh::n_threads());
  _active_boundary_materials.resize(libMesh::n_threads());
}

MaterialWarehouse::~MaterialWarehouse()
{
  for (std::vector<std::map<int, std::vector<Material *> > >::iterator i = _active_materials.begin(); i != _active_materials.end(); ++i)
  {
    for (MaterialIterator j = i->begin(); j != i->end(); ++j)
      for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
        delete (*k);
  }

  for (std::vector<std::map<int, std::vector<Material *> > >::iterator i = _active_boundary_materials.begin(); i != _active_boundary_materials.end(); ++i)
  {
    for (MaterialIterator j = i->begin(); j != i->end(); ++j)
      for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
        delete (*k);
  }
}

std::vector<Material *>
MaterialWarehouse::getMaterials(THREAD_ID tid, unsigned int block_id)
{
  std::stringstream oss;
  
  MaterialIterator mat_iter = _active_materials[tid].find(block_id);
  if (mat_iter == _active_materials[tid].end())
  {
    oss << "Active Material Missing for block: " << block_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *>
MaterialWarehouse::getBoundaryMaterials(THREAD_ID tid, unsigned int boundary_id)
{
  std::stringstream oss;
  
  MaterialIterator mat_iter = _active_boundary_materials[tid].find(boundary_id);
  if (mat_iter == _active_boundary_materials[tid].end())
  {
    oss << "Active Boundary Material Missing for boundary: " << boundary_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

void MaterialWarehouse::updateMaterialDataState()
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    for (MaterialIterator it = _active_materials[tid].begin(); it != _active_materials[tid].end(); ++it)
    {
      for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      {
        (*jt)->updateDataState();
        (*jt)->timeStepSetup();
      }
    }

    for (MaterialIterator it = _active_boundary_materials[tid].begin(); it != _active_boundary_materials[tid].end(); ++it)
    {
      for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      {
        (*jt)->updateDataState();
        (*jt)->timeStepSetup();
      }
    }
  }
}

MaterialIterator
MaterialWarehouse::activeMaterialsBegin(THREAD_ID tid)
{
  return _active_materials[tid].begin();
}

MaterialIterator
MaterialWarehouse::activeMaterialsEnd(THREAD_ID tid)
{
  return _active_materials[tid].end();
}

MaterialIterator
MaterialWarehouse::activeBoundaryMaterialsBegin(THREAD_ID tid)
{
  return _active_boundary_materials[tid].begin();
}

MaterialIterator
MaterialWarehouse::activeBoundaryMaterialsEnd(THREAD_ID tid)
{
  return _active_boundary_materials[tid].end();
}

void
MaterialWarehouse::addMaterial(THREAD_ID tid, int block_id, Material *material)
{
  _active_materials[tid][block_id].push_back(material);
}

void MaterialWarehouse::addBoundaryMaterial(THREAD_ID tid, int block_id, Material *material)
{
  _active_boundary_materials[tid][block_id].push_back(material);
}
