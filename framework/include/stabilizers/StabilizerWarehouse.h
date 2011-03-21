#ifndef STABILIZERWAREHOUSE_H_
#define STABILIZERWAREHOUSE_H_

#include "Stabilizer.h"

/**
 * Typedef to hide implementation details
 */
typedef std::vector<Stabilizer *>::iterator StabilizerIterator;


class StabilizerWarehouse
{
public:
  StabilizerWarehouse();
  virtual ~StabilizerWarehouse();

  bool isStabilized(unsigned int var_num);

  StabilizerIterator activeStabilizersBegin();
  StabilizerIterator activeStabilizersEnd();

  StabilizerIterator blockStabilizersBegin(unsigned int block_id);
  StabilizerIterator blockStabilizersEnd(unsigned int block_id);

  void addStabilizer(Stabilizer *stabilizer);
  void addBlockStabilizer(unsigned int block_id, Stabilizer *stabilizer);

protected:
  std::vector<Stabilizer *> _active_stabilizers;

  std::map<unsigned int, std::vector<Stabilizer *> > _block_stabilizers;

  std::map<unsigned int, bool> _is_stabilized;
};

#endif // STABILIZERWAREHOUSE_H
