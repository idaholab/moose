#ifndef STABILIZERHOLDER_H
#define STABILIZERHOLDER_H

#include "Stabilizer.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<unsigned int, Stabilizer *>::iterator StabilizerIterator;


class StabilizerHolder
{
public:
  StabilizerHolder(MooseSystem &sys);
  virtual ~StabilizerHolder();

  Stabilizer * add(std::string stabilizer_name,
                   std::string name,
                   MooseSystem & moose_system,
                   InputParameters parameters);

  bool isStabilized(unsigned int var_num);

  StabilizerIterator activeStabilizersBegin(THREAD_ID tid);
  StabilizerIterator activeStabilizersEnd(THREAD_ID tid);

  StabilizerIterator blockStabilizersBegin(THREAD_ID tid, unsigned int block_id);
  StabilizerIterator blockStabilizersEnd(THREAD_ID tid, unsigned int block_id);

  bool activeStabilizerBlocks(std::set<subdomain_id_type> & set_buffer) const;

  std::vector<std::map<unsigned int, Stabilizer *> > active_stabilizers;

  std::vector<std::map<unsigned int, std::map<unsigned int, Stabilizer *> > > block_stabilizers;

  std::map<unsigned int, bool> _is_stabilized;

protected:
  MooseSystem &_moose_system;
};

#endif // STABILIZERHOLDER_H
