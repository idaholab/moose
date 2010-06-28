#ifndef BCHOLDER_H
#define BCHOLDER_H

#include <vector>

#include "BoundaryCondition.h"

/**
 * Typedef to hide implementation details
 */
typedef std::vector<BoundaryCondition *>::iterator BCIterator;

class BCHolder
{
public:
  BCHolder(MooseSystem &sys);
  virtual ~BCHolder();

  void sizeEverything();

  BCIterator activeBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  BCIterator activeBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  BCIterator activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  BCIterator activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  void addBC(THREAD_ID tid, unsigned int boundary_id, BoundaryCondition *bc);
  void addNodalBC(THREAD_ID tid, unsigned int boundary_id, BoundaryCondition *bc);

  void activeBoundaries(std::set<subdomain_id_type> & set_buffer) const;

protected:
  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > _active_bcs;
  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > _active_nodal_bcs;

  MooseSystem &_moose_system;
};

#endif // BCHOLDER_H
