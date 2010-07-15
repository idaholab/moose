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
  MaterialWarehouse(MooseSystem &sys);
  virtual ~MaterialWarehouse();

  std::vector<Material *> getMaterials(THREAD_ID tid, unsigned int block_id);
  std::vector<Material *> getBoundaryMaterials(THREAD_ID tid, unsigned int boundary_id);

  void updateMaterialDataState();

  MaterialIterator activeMaterialsBegin(THREAD_ID tid);
  MaterialIterator activeMaterialsEnd(THREAD_ID tid);

  MaterialIterator activeBoundaryMaterialsBegin(THREAD_ID tid);
  MaterialIterator activeBoundaryMaterialsEnd(THREAD_ID tid);

  void addMaterial(THREAD_ID tid, int block_id, Material *material);
  void addBoundaryMaterial(THREAD_ID tid, int block_id, Material *material);

protected:
  /**
   * A list of material associated with the block (subdomain)
   */
  std::vector<std::map<int, std::vector<Material *> > > _active_materials;
  /**
   * A list of boundary materials associated with the block (subdomain)
   */
  std::vector<std::map<int, std::vector<Material *> > > _active_boundary_materials;

  MooseSystem &_moose_system;
};

#endif // MATERIALWAREHOUSE_H
