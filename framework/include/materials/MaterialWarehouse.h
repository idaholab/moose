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

  bool hasMaterials(unsigned int block_id);
  bool hasBoundaryMaterials(unsigned int boundary_id);
  bool hasNeighborMaterials(unsigned int boundary_id);

  std::vector<Material *> & getMaterials(unsigned int block_id);
  std::vector<Material *> & getBoundaryMaterials(unsigned int boundary_id);
  std::vector<Material *> & getNeighborMaterials(unsigned int boundary_id);

  MaterialIterator activeMaterialsBegin() { return _active_materials.begin(); }
  MaterialIterator activeMaterialsEnd() { return _active_materials.end(); }
  
  void updateMaterialDataState();

  void addMaterial(int block_id, Material *material);
  void addBoundaryMaterial(int block_id, Material *material);
  void addNeighborMaterial(int block_id, Material *material);

protected:
  /**
   * A list of material associated with the block (subdomain)
   */
  std::map<int, std::vector<Material *> > _active_materials;
  /**
   * A list of boundary materials associated with the block (subdomain)
   */
  std::map<int, std::vector<Material *> > _active_boundary_materials;
  /**
   * A list of neighbor materials associated with the block (subdomain) (for DG)
   */
  std::map<int, std::vector<Material *> > _active_neighbor_materials;
};

#endif // MATERIALWAREHOUSE_H
