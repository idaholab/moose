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

  std::vector<Material *> & getMaterials(unsigned int block_id);
  std::vector<Material *> & getBoundaryMaterials(unsigned int boundary_id);

  void updateMaterialDataState();

  MaterialIterator activeMaterialsBegin();
  MaterialIterator activeMaterialsEnd();

  MaterialIterator activeBoundaryMaterialsBegin();
  MaterialIterator activeBoundaryMaterialsEnd();

  void addMaterial(int block_id, Material *material);
  void addBoundaryMaterial(int block_id, Material *material);

protected:
  /**
   * A list of material associated with the block (subdomain)
   */
  std::map<int, std::vector<Material *> > _active_materials;
  /**
   * A list of boundary materials associated with the block (subdomain)
   */
  std::map<int, std::vector<Material *> > _active_boundary_materials;
};

#endif // MATERIALWAREHOUSE_H
