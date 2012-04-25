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

#ifndef MATERIALWAREHOUSE_H
#define MATERIALWAREHOUSE_H

#include <vector>
#include <map>

#include "Material.h"

// Forward Declaration
template <class T> class DependencyResolver;

class MaterialWarehouse
{
public:
  MaterialWarehouse();

  // Copy Constructor
  MaterialWarehouse(const MaterialWarehouse &rhs);

  virtual ~MaterialWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  bool hasMaterials(SubdomainID block_id);
  bool hasBoundaryMaterials(BoundaryID boundary_id);
  bool hasNeighborMaterials(BoundaryID boundary_id);

  std::vector<Material *> & getMaterialsByName(const std::string & name);

  std::vector<Material *> & getMaterials(SubdomainID block_id);
  std::vector<Material *> & getBoundaryMaterials(BoundaryID boundary_id);
  std::vector<Material *> & getNeighborMaterials(BoundaryID boundary_id);

  const std::vector<Material *> & active(SubdomainID block_id) { return _active_materials[block_id]; }

  void updateMaterialDataState();

  void addMaterial(SubdomainID block_id, Material *material);
  void addBoundaryMaterial(SubdomainID block_id, Material *material);
  void addNeighborMaterial(SubdomainID block_id, Material *material);

  /**
   * Get the list of blocks that materials are defined on
   * @return The list of subdomain IDs
   */
  const std::set<SubdomainID> & blocks() { return _blocks; }

protected:
  /// A list of material associated with the block (subdomain)
  std::map<SubdomainID, std::vector<Material *> > _active_materials;
  /// A list of boundary materials associated with the block (subdomain)
  std::map<SubdomainID, std::vector<Material *> > _active_boundary_materials;
  /// A list of neighbor materials associated with the block (subdomain) (for DG)
  std::map<SubdomainID, std::vector<Material *> > _active_neighbor_materials;

  /// Set of blocks where materials are defined
  std::set<SubdomainID> _blocks;

  /// A convenience list of all the maps
  std::vector<std::map<SubdomainID, std::vector<Material *> > *> _master_list;

  /// list of materials by name
  std::map<std::string, std::vector<Material *> > _mat_by_name;

private:
  /**
   * This routine uses the Dependency Resolver to sort Materials based on dependencies they
   * might have on coupled values
   */
  void sortMaterials(std::map<SubdomainID, std::vector<Material *> > & materials_map);
};

#endif // MATERIALWAREHOUSE_H
