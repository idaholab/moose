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
MaterialWarehouse::initialSetup()
{
  sortMaterials();

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
MaterialWarehouse::sortMaterials()
{
#ifdef DEBUG
  // Sanity check that all three containers contain the same keys (well almost we aren't checking "every" case)
  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin();
       j != _active_materials.end(); ++j)
  {
    mooseAssert(_active_boundary_materials.find(j->first) != _active_boundary_materials.end(),
                "active_boundary_materials is different then active_materials");

    // TODO: Enable this check when doing DG
    //mooseAssert(_active_neighbor_materials.find(j->first) != _active_neighbor_materials.end(),
    //             "active_neighbor_materials is different then active_materials");
  }
#endif

  for (std::map<int, std::vector<Material *> >::iterator j = _active_materials.begin(); j != _active_materials.end(); ++j)
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

      for (std::set<std::string>::const_iterator prop_iter=depend_props.begin(); prop_iter != depend_props.end(); ++prop_iter)
      {
        // Ask each active material if they supply this property
        for (std::vector<Material *>::const_iterator mat_iter2=j->second.begin(); mat_iter2 != j->second.end(); ++mat_iter2)
        {
          // Don't check THIS material for a coupled property
          if (mat_iter == mat_iter2) continue;

          if ((*mat_iter2)->have_property_name(*prop_iter))
            resolver.insertDependency(*mat_iter, *mat_iter2);
        }
      }
    }

    // Sort based on dependencies - Can we use the same resolver for all three material data structures? I think so!
    std::sort(j->second.begin(), j->second.end(), resolver);

    std::sort(_active_boundary_materials[j->first].begin(), _active_boundary_materials[j->first].end(), resolver);

    // TODO: Sort this list when doing DG
    //std::sort(_active_neighbor_materials[j->first].begin(), _active_neighbor_materials[j->first].end(), resolver);
  }

}
