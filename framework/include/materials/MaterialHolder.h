#ifndef MATERIALHOLDER_H
#define MATERIALHOLDER_H

#include <vector>
#include <map>

#include "Material.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<int, Material *>::iterator MaterialIterator;


class MaterialHolder
{
public:
  MaterialHolder(MooseSystem &sys);
  virtual ~MaterialHolder();

  Material * getMaterial(THREAD_ID tid, unsigned int block_id);

  void updateMaterialDataState();

  MaterialIterator activeMaterialsBegin(THREAD_ID tid);
  MaterialIterator activeMaterialsEnd(THREAD_ID tid);

  std::vector<std::map<int, Material *> > active_materials;

protected:
  MooseSystem &_moose_system;
};

#endif // MATERIALHOLDER_H
