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
#include <set>

#include "MooseTypes.h"


class Material;

// Forward Declaration
template <class T> class DependencyResolver;

/**
 * Stores materials.
 *
 * There are currently these types of materials
 * 1) material - material defined in the interior of an element (associated with a subdomain)
 * 2) face material - same as above but the material lives on the element side (also associated with a subdomain)
 * 3) neighbor material - same as face material but on the neighboring element (used by DG)
 * 4) boundary material - defined on a surface (associated with a side set)
 */
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
  bool hasFaceMaterials(SubdomainID block_id);
  bool hasNeighborMaterials(SubdomainID block_id);
  bool hasBoundaryMaterials(BoundaryID boundary_id);

  std::vector<Material *> & getMaterialsByName(const std::string & name);

  std::vector<Material *> & getMaterials(SubdomainID block_id);
  std::vector<Material *> & getFaceMaterials(SubdomainID block_id);
  std::vector<Material *> & getNeighborMaterials(SubdomainID block_id);
  std::vector<Material *> & getBoundaryMaterials(BoundaryID boundary_id);

  const std::vector<Material *> & active(SubdomainID block_id) { return _active_materials[block_id]; }

  void updateMaterialDataState();

  void addMaterial(std::vector<SubdomainID> blocks, Material *material);
  void addFaceMaterial(std::vector<SubdomainID> blocks, Material *material);
  void addNeighborMaterial(std::vector<SubdomainID> blocks, Material *material);
  void addBoundaryMaterial(std::vector<BoundaryID> boundaries, Material *material);

  /**
   * Get the list of blocks that materials are defined on
   * @return The list of subdomain IDs
   */
  const std::set<SubdomainID> & blocks() const { return _blocks; }

  /**
   * Get the list of boundary ids that the materials are defined
   * @return The set of Boundary IDs
   */
  const std::set<BoundaryID> & boundaries() const { return _boundaries; }

  /// This method displays a list of active materials and the properties they supply
  void printMaterialMap() const;

  /// This method checks for coupled material properties to make sure that all retrieved properties are supplied
  void checkMaterialDependSanity() const;

  /// This method loops over all materials and calls checkStatefulSanity() on the individual materials
  void checkStatefulSanity() const;

protected:
  /// A list of material associated with the block (subdomain)
  std::map<SubdomainID, std::vector<Material *> > _active_materials;

  /// A list of face materials associated with the block (subdomain)
  std::map<SubdomainID, std::vector<Material *> > _active_face_materials;

  /// A list of neighbor materials associated with the block (subdomain) (for DG)
  std::map<SubdomainID, std::vector<Material *> > _active_neighbor_materials;

  /// A list of boundary materials associated with the boundary (boundary)
  std::map<BoundaryID, std::vector<Material *> > _active_boundary_materials;

  /// Set of blocks where materials are defined
  std::set<SubdomainID> _blocks;

  /// Set of boundaries where materials are defined
  std::set<BoundaryID> _boundaries;

  /// A convenience list of all the maps
  std::vector<std::map<SubdomainID, std::vector<Material *> > *> _master_list;

  /// list of materials by name
  std::map<std::string, std::vector<Material *> > _mat_by_name;

  /// All of the material objects this warehouse knows about
  std::vector<Material *> _mats;

private:
  /**
   * This routine uses the Dependency Resolver to sort Materials based on dependencies they
   * might have on coupled values
   */
  void sortMaterials(std::vector<Material *> & materials_vector);

  /**
   * This routine checks to make sure that all requests for material properties, specifically
   * by other materials make sense for the given block.
   */
  void checkDependMaterials(const std::map<SubdomainID, std::vector<Material *> > & materials_map) const;
};

#endif // MATERIALWAREHOUSE_H
