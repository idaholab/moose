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

  void activeBoundaries(std::set<subdomain_id_type> & set_buffer) const;

  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > active_bcs;
  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > active_nodal_bcs;

protected:
  MooseSystem &_moose_system;
};

#endif // BCHOLDER_H
