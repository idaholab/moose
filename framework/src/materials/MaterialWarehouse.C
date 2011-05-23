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
#include "DependencyResolver.h"

// FIXME: !
#define unlikely(a) a

MaterialWarehouse::MaterialWarehouse()
{
}

MaterialWarehouse::~MaterialWarehouse()
{
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      delete (*k);

  for (std::map<int, std::vector<Material *> >::iterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      delete (*k);

  for (std::map<int, std::vector<Material *> >::iterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      delete (*k);
}


void
MaterialWarehouse::initialSetup(DependencyResolver<std::string> & _mat_prop_depends)
{
  sortMaterials(_mat_prop_depends);
  
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->initialSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->initialSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->initialSetup();
}

void
MaterialWarehouse::timestepSetup()
{
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->timestepSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->timestepSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->timestepSetup();
}

void
MaterialWarehouse::residualSetup()
{
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->residualSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->residualSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->residualSetup();
}

void
MaterialWarehouse::jacobianSetup()
{
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->jacobianSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
    for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
      (*k)->jacobianSetup();

  for (std::map<int, std::vector<Material *> >::iterator j = _active_neighbor_materials.begin(); j != _active_neighbor_materials.end(); ++j)
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
  std::map<int, std::vector<Material *> >::iterator mat_iter = _active_materials.find(block_id);
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
  std::map<int, std::vector<Material *> >::iterator mat_iter = _active_boundary_materials.find(boundary_id);
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
  std::map<int, std::vector<Material *> >::iterator mat_iter = _active_neighbor_materials.find(boundary_id);
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
  for (std::map<int, std::vector<Material *> >::iterator it = _active_materials.begin(); it != _active_materials.end(); ++it)
  {
    for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      (*jt)->updateDataState();
      (*jt)->timeStepSetup();
    }
  }

  for (std::map<int, std::vector<Material *> >::iterator it = _active_boundary_materials.begin(); it != _active_boundary_materials.end(); ++it)
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
  _blocks.insert(block_id);
  _active_materials[block_id].push_back(material);
}

void MaterialWarehouse::addBoundaryMaterial(int block_id, Material *material)
{
  _blocks.insert(block_id);
  _active_boundary_materials[block_id].push_back(material);
}

void MaterialWarehouse::addNeighborMaterial(int block_id, Material *material)
{
  _blocks.insert(block_id);
  _active_neighbor_materials[block_id].push_back(material);
}

void
MaterialWarehouse::sortMaterials(DependencyResolver<std::string> & _mat_prop_depends)
{
  std::map<int, std::vector<Material *> > new_order;
  
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)  
  {
    const std::vector<std::string> & sorted_names = _mat_prop_depends.getSortedValues();

    for (unsigned int i=0; i<sorted_names.size(); ++i)
      std::cout << sorted_names[i] << "\n";
    
    std::vector<Material *> *materials = new std::vector<Material *>();

    for (std::vector<std::string>::const_iterator name_iter=sorted_names.begin(); name_iter != sorted_names.end(); ++name_iter)
      for (std::vector<Material *>::iterator mat_iter=j->second.begin(); mat_iter != j->second.end(); ++mat_iter)
        if (*name_iter == (*mat_iter)->name())
        {
          materials->push_back(*mat_iter);
        }
    
    // grab the materials that don't have any depends and put them in the front
    for (std::vector<Material *>::iterator mat_iter=j->second.begin(); mat_iter != j->second.end(); ++mat_iter)
      if (std::find(materials->begin(), materials->end(), *mat_iter) == materials->end())
        materials->insert(materials->begin(), *mat_iter);

    new_order[j->first] = *materials;
  }

  // Swap out the new order with the old order

  // TODO: this isn't done yet!
  _active_materials.swap(new_order);
}
