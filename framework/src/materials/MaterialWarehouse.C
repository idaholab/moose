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
  _master_list.reserve(3);
  _master_list.push_back(&_active_materials);
  _master_list.push_back(&_active_boundary_materials);
  _master_list.push_back(&_active_neighbor_materials);
}

MaterialWarehouse::MaterialWarehouse(const MaterialWarehouse &rhs)
{
  _active_materials = rhs._active_materials;
  _active_boundary_materials = rhs._active_boundary_materials;
  _active_neighbor_materials = rhs._active_neighbor_materials;
  _blocks = rhs._blocks;

  _master_list.reserve(3);
  _master_list.push_back(&_active_materials);
  _master_list.push_back(&_active_boundary_materials);
  _master_list.push_back(&_active_neighbor_materials);
}


MaterialWarehouse::~MaterialWarehouse()
{
  for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    for (std::map<SubdomainID, std::vector<Material *> >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
        delete (*k);
}


void
MaterialWarehouse::initialSetup()
{
    for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    {
      sortMaterials(**i);
      for (std::map<SubdomainID, std::vector<Material *> >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
        for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
          (*k)->initialSetup();
    }
}

void
MaterialWarehouse::timestepSetup()
{
  for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    for (std::map<SubdomainID, std::vector<Material *> >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
        (*k)->timestepSetup();
}

void
MaterialWarehouse::residualSetup()
{
  for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    for (std::map<SubdomainID, std::vector<Material *> >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
        (*k)->residualSetup();
}

void
MaterialWarehouse::jacobianSetup()
{
  for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    for (std::map<SubdomainID, std::vector<Material *> >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      for (std::vector<Material *>::iterator k = j->second.begin(); k != j->second.end(); ++k)
        (*k)->jacobianSetup();
}

bool
MaterialWarehouse::hasMaterials(SubdomainID block_id)
{
  return (_active_materials.find(block_id) != _active_materials.end());
}

bool
MaterialWarehouse::hasBoundaryMaterials(BoundaryID boundary_id)
{
  return (_active_boundary_materials.find(boundary_id) != _active_boundary_materials.end());
}

bool
MaterialWarehouse::hasNeighborMaterials(BoundaryID boundary_id)
{
  return (_active_neighbor_materials.find(boundary_id) != _active_neighbor_materials.end());
}

std::vector<Material *> &
MaterialWarehouse::getMaterials(SubdomainID block_id)
{
  std::map<SubdomainID, std::vector<Material *> >::iterator mat_iter = _active_materials.find(block_id);
  if (unlikely(mat_iter == _active_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Material Missing for block: " << block_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *> &
MaterialWarehouse::getBoundaryMaterials(BoundaryID boundary_id)
{
  std::map<SubdomainID, std::vector<Material *> >::iterator mat_iter = _active_boundary_materials.find(boundary_id);
  if (unlikely(mat_iter == _active_boundary_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Boundary Material Missing for boundary: " << boundary_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *> &
MaterialWarehouse::getNeighborMaterials(BoundaryID boundary_id)
{
  std::map<SubdomainID, std::vector<Material *> >::iterator mat_iter = _active_neighbor_materials.find(boundary_id);
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
  for (std::map<SubdomainID, std::vector<Material *> >::iterator it = _active_materials.begin(); it != _active_materials.end(); ++it)
  {
    for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      (*jt)->timeStepSetup();
    }
  }

  for (std::map<SubdomainID, std::vector<Material *> >::iterator it = _active_boundary_materials.begin(); it != _active_boundary_materials.end(); ++it)
  {
    for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      (*jt)->timeStepSetup();
    }
  }
}

void
MaterialWarehouse::addMaterial(SubdomainID block_id, Material *material)
{
  _blocks.insert(block_id);
  _active_materials[block_id].push_back(material);
  _mat_by_name[material->name()].push_back(material);
}

void MaterialWarehouse::addBoundaryMaterial(SubdomainID block_id, Material *material)
{
  _blocks.insert(block_id);
  _active_boundary_materials[block_id].push_back(material);
  _mat_by_name[material->name()].push_back(material);
}

void MaterialWarehouse::addNeighborMaterial(SubdomainID block_id, Material *material)
{
  _blocks.insert(block_id);
  _active_neighbor_materials[block_id].push_back(material);
  _mat_by_name[material->name()].push_back(material);
}

void
MaterialWarehouse::sortMaterials(std::map<SubdomainID, std::vector<Material *> > & materials_map)
{
  for (std::map<SubdomainID, std::vector<Material *> >::iterator j = materials_map.begin(); j != materials_map.end(); ++j)
  {
    DependencyResolver<Material *> resolver;

    /**
     * For each block we have to run through the dependency list since
     * the property provided by a material can change from block to block
     * which can change the dependencies
     */
    for (std::vector<Material *>::const_iterator mat_iter=j->second.begin(); mat_iter != j->second.end(); ++mat_iter)
    {
      const std::set<std::string> & depend_props = (*mat_iter)->getPropertyDependencies();

      // See if any of the active materials supply this property
      for (std::vector<Material *>::const_iterator mat_iter2=j->second.begin(); mat_iter2 != j->second.end(); ++mat_iter2)
      {
        // Don't check THIS material for a coupled property
        if (mat_iter == mat_iter2) continue;

        const std::set<std::string> & supplied_props = (*mat_iter2)->getSuppliedPropertiesList();

        std::set<std::string> intersect;
        std::set_intersection(depend_props.begin(), depend_props.end(), supplied_props.begin(),
                              supplied_props.end(), std::inserter(intersect, intersect.end()));

        // If the intersection isn't empty then there is a dependency here
        if (!intersect.empty())
          resolver.insertDependency(*mat_iter, *mat_iter2);
      }
    }

    // Sort based on dependencies
    std::sort(j->second.begin(), j->second.end(), resolver);
  }
}

std::vector<Material *> &
MaterialWarehouse::getMaterialsByName(const std::string & name)
{
  if (_mat_by_name.find(name) == _mat_by_name.end())
    mooseError("Could not find material with name '" << name << "'");
  return _mat_by_name[name];
}
