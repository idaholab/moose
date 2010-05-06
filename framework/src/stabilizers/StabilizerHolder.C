#include "StabilizerHolder.h"

StabilizerHolder::StabilizerHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  active_stabilizers.resize(libMesh::n_threads());
  block_stabilizers.resize(libMesh::n_threads());
}

StabilizerHolder::~StabilizerHolder()
{
  {

    std::vector<std::map<unsigned int, Stabilizer *> >::iterator i;
    for (i=active_stabilizers.begin(); i!=active_stabilizers.end(); ++i)
    {

      StabilizerIterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        delete j->second;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::map<unsigned int, Stabilizer *> > >::iterator i;
    for (i=block_stabilizers.begin(); i!=block_stabilizers.end(); ++i)
    {

      std::map<unsigned int, std::map<unsigned int, Stabilizer *> >::iterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        StabilizerIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete k->second;
        }
      }
    }
  }
}

bool
StabilizerHolder::isStabilized(unsigned int var_num)
{
  return _is_stabilized[var_num];
}

StabilizerIterator
StabilizerHolder::activeStabilizersBegin(THREAD_ID tid)
{
  return active_stabilizers[tid].begin();
}

StabilizerIterator
StabilizerHolder::activeStabilizersEnd(THREAD_ID tid)
{
  return active_stabilizers[tid].end();
}


StabilizerIterator
StabilizerHolder::blockStabilizersBegin(THREAD_ID tid, unsigned int block_id)
{
  return block_stabilizers[tid][block_id].begin();
}

StabilizerIterator
StabilizerHolder::blockStabilizersEnd(THREAD_ID tid, unsigned int block_id)
{
  return block_stabilizers[tid][block_id].end();
}
