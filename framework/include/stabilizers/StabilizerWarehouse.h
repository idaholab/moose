#ifndef STABILIZERWAREHOUSE_H
#define STABILIZERWAREHOUSE_H

#include "Stabilizer.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<unsigned int, Stabilizer *>::iterator StabilizerIterator;


class StabilizerWarehouse
{
public:
  StabilizerWarehouse(MooseSystem &sys);
  virtual ~StabilizerWarehouse();

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

  void addBlockStabilizer(THREAD_ID tid, unsigned int block_id, unsigned int var_num, Stabilizer *stabilizer);
  void addStabilizer(THREAD_ID tid, unsigned int var_num, Stabilizer *stabilizer);

protected:
  std::vector<std::map<unsigned int, Stabilizer *> > _active_stabilizers;

  std::vector<std::map<unsigned int, std::map<unsigned int, Stabilizer *> > > _block_stabilizers;

  std::map<unsigned int, bool> _is_stabilized;

  MooseSystem &_moose_system;
};

#endif // STABILIZERWAREHOUSE_H
