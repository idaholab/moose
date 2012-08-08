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
#include "DependencyResolverInterface.h"
#include "Material.h"

#include <fstream>

// FIXME: !
#define unlikely(a) a

MaterialWarehouse::MaterialWarehouse()
{
  _master_list.reserve(3);
  _master_list.push_back(&_active_materials);
  _master_list.push_back(&_active_face_materials);
  _master_list.push_back(&_active_neighbor_materials);
}

MaterialWarehouse::MaterialWarehouse(const MaterialWarehouse &rhs)
{
  _active_materials = rhs._active_materials;
  _active_face_materials = rhs._active_face_materials;
  _active_neighbor_materials = rhs._active_neighbor_materials;
  _active_boundary_materials = rhs._active_boundary_materials;
  _blocks = rhs._blocks;
  _boundaries = rhs._boundaries;

  _master_list.reserve(3);
  _master_list.push_back(&_active_materials);
  _master_list.push_back(&_active_face_materials);
  _master_list.push_back(&_active_neighbor_materials);
}


MaterialWarehouse::~MaterialWarehouse()
{
  for(unsigned int i=0; i<_mats.size(); i++)
    delete _mats[i];
}


void
MaterialWarehouse::initialSetup()
{
  // Sort the materials in each subdomain to get correct execution order
  for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    for (std::map<SubdomainID, std::vector<Material *> >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      sortMaterials(j->second);

  for (std::map<BoundaryID, std::vector<Material *> >::iterator j = _active_boundary_materials.begin(); j != _active_boundary_materials.end(); ++j)
     sortMaterials(j->second);

  for(unsigned int i=0; i<_mats.size(); i++)
    _mats[i]->initialSetup();
}

void
MaterialWarehouse::timestepSetup()
{
  for(unsigned int i=0; i<_mats.size(); i++)
    _mats[i]->timestepSetup();
}

void
MaterialWarehouse::residualSetup()
{
  for(unsigned int i=0; i<_mats.size(); i++)
    _mats[i]->residualSetup();
}

void
MaterialWarehouse::jacobianSetup()
{
  for(unsigned int i=0; i<_mats.size(); i++)
    _mats[i]->jacobianSetup();
}

bool
MaterialWarehouse::hasMaterials(SubdomainID block_id)
{
  return (_active_materials.find(block_id) != _active_materials.end());
}

bool
MaterialWarehouse::hasFaceMaterials(SubdomainID block_id)
{
  return (_active_face_materials.find(block_id) != _active_face_materials.end());
}

bool
MaterialWarehouse::hasNeighborMaterials(SubdomainID block_id)
{
  return (_active_neighbor_materials.find(block_id) != _active_neighbor_materials.end());
}

bool
MaterialWarehouse::hasBoundaryMaterials(BoundaryID boundary_id)
{
  return (_active_boundary_materials.find(boundary_id) != _active_boundary_materials.end());
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
MaterialWarehouse::getFaceMaterials(SubdomainID block_id)
{
  std::map<SubdomainID, std::vector<Material *> >::iterator mat_iter = _active_face_materials.find(block_id);
  if (unlikely(mat_iter == _active_face_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Face Material Missing for block: " << block_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *> &
MaterialWarehouse::getNeighborMaterials(SubdomainID block_id)
{
  std::map<SubdomainID, std::vector<Material *> >::iterator mat_iter = _active_neighbor_materials.find(block_id);
  if (unlikely(mat_iter == _active_neighbor_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Neighbor Material Missing for block: " << block_id << "\n";
    mooseError(oss.str());
  }
  return mat_iter->second;
}

std::vector<Material *> &
MaterialWarehouse::getBoundaryMaterials(BoundaryID boundary_id)
{
  std::map<BoundaryID, std::vector<Material *> >::iterator mat_iter = _active_boundary_materials.find(boundary_id);
  if (unlikely(mat_iter == _active_boundary_materials.end()))
  {
    std::stringstream oss;
    oss << "Active Boundary Material Missing for boundary: " << boundary_id << "\n";
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

  for (std::map<SubdomainID, std::vector<Material *> >::iterator it = _active_face_materials.begin(); it != _active_face_materials.end(); ++it)
  {
    for (std::vector<Material *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      (*jt)->timeStepSetup();
    }
  }
}

void
MaterialWarehouse::addMaterial(std::vector<SubdomainID> blocks, Material *material)
{
  _mats.push_back(material);

  for (unsigned int i=0; i<blocks.size(); ++i)
  {
    SubdomainID blk_id = blocks[i];
    _active_materials[blk_id].push_back(material);
    _mat_by_name[material->name()].push_back(material);
  }
}

void MaterialWarehouse::addFaceMaterial(std::vector<SubdomainID> blocks, Material *material)
{
  _mats.push_back(material);

  for (unsigned int i=0; i<blocks.size(); ++i)
  {
    SubdomainID blk_id = blocks[i];
    _blocks.insert(blk_id);
    _active_face_materials[blk_id].push_back(material);
    _mat_by_name[material->name()].push_back(material);
  }
}

void MaterialWarehouse::addNeighborMaterial(std::vector<SubdomainID> blocks, Material *material)
{
  _mats.push_back(material);

  for (unsigned int i=0; i<blocks.size(); ++i)
  {
    SubdomainID blk_id = blocks[i];
    _blocks.insert(blk_id);
    _active_neighbor_materials[blk_id].push_back(material);
    _mat_by_name[material->name()].push_back(material);
  }
}

void MaterialWarehouse::addBoundaryMaterial(std::vector<BoundaryID> boundaries, Material *material)
{
  _mats.push_back(material);

  for (unsigned int i = 0; i < boundaries.size(); ++i)
  {
    SubdomainID bnd_id = boundaries[i];
    _boundaries.insert(bnd_id);
    _active_boundary_materials[bnd_id].push_back(material);
    _mat_by_name[material->name()].push_back(material);
  }
}

void
MaterialWarehouse::checkMaterialDependSanity()
{
  for (std::vector<std::map<SubdomainID, std::vector<Material *> > *>::iterator i = _master_list.begin(); i != _master_list.end(); ++i)
    checkDependMaterials(**i);
}

void
MaterialWarehouse::checkDependMaterials(std::map<SubdomainID, std::vector<Material *> > & materials_map)
{
  for (std::map<SubdomainID, std::vector<Material *> >::iterator j = materials_map.begin(); j != materials_map.end(); ++j)
  {
    /// These two sets are used to make sure that all depenedent props on a block are actually supplied
    std::set<std::string> block_depend_props, block_supplied_props;

    for (std::vector<Material *>::const_iterator mat_iter=j->second.begin(); mat_iter != j->second.end(); ++mat_iter)
    {
      const std::set<std::string> & depend_props = (*mat_iter)->getRequestedItems();
      block_depend_props.insert(depend_props.begin(), depend_props.end());

      // See if any of the active materials supply this property
      for (std::vector<Material *>::const_iterator mat_iter2=j->second.begin(); mat_iter2 != j->second.end(); ++mat_iter2)
      {
        // Don't check THIS material for a coupled property
        if (mat_iter == mat_iter2) continue;

        const std::set<std::string> & supplied_props = (*mat_iter2)->getSuppliedItems();
        block_supplied_props.insert(supplied_props.begin(), supplied_props.end());
      }
    }

    // Error check to make sure all propoerites consumed by materials are supplied on this block
    std::set<std::string> difference;
    std::set_difference(block_depend_props.begin(), block_depend_props.end(), block_supplied_props.begin(), block_supplied_props.end(),
                        std::inserter(difference, difference.end()));

    if (!difference.empty())
    {
      std::ostringstream oss;
      oss << "One or more Material Properties were not supplied on block " << j->first << ":\n";
      for (std::set<std::string>::iterator i = difference.begin(); i != difference.end();  ++i)
        oss << *i << "\n";
      mooseError(oss.str());
    }
  }
}

void
MaterialWarehouse::sortMaterials(std::vector<Material *> & materials_vector)
{
  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort(materials_vector.begin(), materials_vector.end());
  }
  catch(CyclicDependencyException<DependencyResolverInterface *> & e)
  {
    std::ostringstream oss;

    oss << "Cyclic dependency detected in material property couplings:\n";
    const std::multimap<DependencyResolverInterface *, DependencyResolverInterface *> & depends = e.getCyclicDependencies();
    for (std::multimap<DependencyResolverInterface *, DependencyResolverInterface *>::const_iterator it = depends.begin(); it != depends.end(); ++it)
      oss << (static_cast<Material *>(it->first))->name() << " -> " << (static_cast<Material *>(it->first))->name() << "\n";
    mooseError(oss.str());
  }
}

std::vector<Material *> &
MaterialWarehouse::getMaterialsByName(const std::string & name)
{
  if (_mat_by_name.find(name) == _mat_by_name.end())
    mooseError("Could not find material with name '" << name << "'");
  return _mat_by_name[name];
}
