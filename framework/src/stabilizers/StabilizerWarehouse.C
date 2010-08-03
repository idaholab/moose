#include "StabilizerWarehouse.h"

StabilizerWarehouse::StabilizerWarehouse()
{
}

StabilizerWarehouse::~StabilizerWarehouse()
{
  for (StabilizerIterator j=_active_stabilizers.begin(); j!=_active_stabilizers.end(); ++j)
    delete j->second;

  for (std::map<unsigned int, std::map<unsigned int, Stabilizer *> >::iterator j=_block_stabilizers.begin(); j!=_block_stabilizers.end(); ++j)
  {
    StabilizerIterator k;
    for(k=(j->second).begin(); k!=(j->second).end(); ++k)
      delete k->second;
  }
}

bool
StabilizerWarehouse::isStabilized(unsigned int var_num)
{
  return _is_stabilized[var_num];
}

StabilizerIterator
StabilizerWarehouse::activeStabilizersBegin()
{
  return _active_stabilizers.begin();
}

StabilizerIterator
StabilizerWarehouse::activeStabilizersEnd()
{
  return _active_stabilizers.end();
}


StabilizerIterator
StabilizerWarehouse::blockStabilizersBegin(unsigned int block_id)
{
  return _block_stabilizers[block_id].begin();
}

StabilizerIterator
StabilizerWarehouse::blockStabilizersEnd(unsigned int block_id)
{
  return _block_stabilizers[block_id].end();
}

void
StabilizerWarehouse::addBlockStabilizer(unsigned int block_id, unsigned int var_num, Stabilizer *stabilizer)
{
  _block_stabilizers[block_id][var_num] = stabilizer;
  _is_stabilized[var_num] = true;
}

void
StabilizerWarehouse::addStabilizer(unsigned int var_num, Stabilizer *stabilizer)
{
  _active_stabilizers[var_num] = stabilizer;
  _is_stabilized[var_num] = true;
}
