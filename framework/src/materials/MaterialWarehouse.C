/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaterialWarehouse.h"

// FIXME: !
#define unlikely(a) a

MaterialWarehouse::MaterialWarehouse()
{
}

MaterialWarehouse::~MaterialWarehouse()
{
  for (MaterialIterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      delete (*k);

  for (MaterialIterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      delete (*k);

  for (MaterialIterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      delete (*k);
}


void
MaterialWarehouse::initialSetup()
{
  for (MaterialIterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->initialSetup();

  for (MaterialIterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->initialSetup();

  for (MaterialIterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->initialSetup();
}

void
MaterialWarehouse::timestepSetup()
{
  for (MaterialIterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->timestepSetup();

  for (MaterialIterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->timestepSetup();

  for (MaterialIterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->timestepSetup();
}

void
MaterialWarehouse::residualSetup()
{
  for (MaterialIterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->residualSetup();

  for (MaterialIterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->residualSetup();

  for (MaterialIterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->residualSetup();
}

void
MaterialWarehouse::jacobianSetup()
{
  for (MaterialIterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->jacobianSetup();

  for (MaterialIterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->jacobianSetup();

  for (MaterialIterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->jacobianSetup();
}

bool
MaterialWarehouse::hasMaterials(unsigned int block_id)
{
  return (_active_materials.find(block_id) != _active_materials.end());
}

bool
MaterialWarehouse::hasBoundaryMaterials(unsigned int boundary_id)
{
  return (_active_boundary_materials.find(boundary_id) != _active_boundary_materials.end());
}

bool
MaterialWarehouse::hasNeighborMaterials(unsigned int boundary_id)
{
  return (_active_neighbor_materials.find(boundary_id) != _active_neighbor_materials.end());
}

std::vector<Material *> &
MaterialWarehouse::getMaterials(unsigned int block_id)
{
  MaterialIterator mat_iter = _active_materials.find(block_id);
  if (unlikely(mat_iter == _active_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Material Missing for block: " << block_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *> &
MaterialWarehouse::getBoundaryMaterials(unsigned int boundary_id)
{
  MaterialIterator mat_iter = _active_boundary_materials.find(boundary_id);
  if (unlikely(mat_iter == _active_boundary_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Boundary Material Missing for boundary: " << boundary_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *> &
MaterialWarehouse::getNeighborMaterials(unsigned int boundary_id)
{
  MaterialIterator mat_iter = _active_neighbor_materials.find(boundary_id);
  if (unlikely(mat_iter == _active_neighbor_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Neighbor Material Missing for boundary: " << boundary_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

void MaterialWarehouse::updateMaterialDataState()
{
  for (MaterialIterator it = _active_materials.begin(); it != _active_materials.end(); ++it)
  {
    for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      (*jt)->updateDataState();
      (*jt)->timeStepSetup();
    }
  }

  for (MaterialIterator it = _active_boundary_materials.begin(); it != _active_boundary_materials.end(); ++it)
  {
    for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      (*jt)->updateDataState();
      (*jt)->timeStepSetup();
    }
  }
}

void
MaterialWarehouse::addMaterial(int block_id, Material *material)
{
  _active_materials[block_id].push_back(material);
}

void MaterialWarehouse::addBoundaryMaterial(int block_id, Material *material)
{
  _active_boundary_materials[block_id].push_back(material);
}

void MaterialWarehouse::addNeighborMaterial(int block_id, Material *material)
{
  _active_neighbor_materials[block_id].push_back(material);
}

