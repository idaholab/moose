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

/**
 * Typedef to hide implementation details
 */
typedef std::map<int, std::vector<Material *> >::iterator MaterialIterator;


class MaterialWarehouse
{
public:
  MaterialWarehouse();
  virtual ~MaterialWarehouse();

  std::vector<Material *> & getMaterials(subdomain_id_type block_id);
  std::vector<Material *> & getBoundaryMaterials(subdomain_id_type block_id);
  std::vector<Material *> & getNeighborMaterials(subdomain_id_type block_id);

  void updateMaterialDataState();

  MaterialIterator activeMaterialsBegin();
  MaterialIterator activeMaterialsEnd();

  MaterialIterator activeBoundaryMaterialsBegin();
  MaterialIterator activeBoundaryMaterialsEnd();

  MaterialIterator activeNeighborMaterialsBegin();
  MaterialIterator activeNeighborMaterialsEnd();

  void addMaterial(int block_id, Material *material);
  void addBoundaryMaterial(int block_id, Material *material);
  void addNeighborMaterial(int block_id, Material *material);

protected:
  /**
   * A list of material associated with the block (subdomain)
   */
  std::map<subdomain_id_type, std::vector<Material *> > _active_materials;
  /**
   * A list of boundary materials associated with the block (subdomain)
   */
  std::map<subdomain_id_type, std::vector<Material *> > _active_boundary_materials;
  /**
   * A list of neighbor materials associated with the block (subdomain) (for DG)
   */
  std::map<subdomain_id_type, std::vector<Material *> > _active_neighbor_materials;
};

#endif // MATERIALWAREHOUSE_H
