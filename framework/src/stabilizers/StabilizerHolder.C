#include "StabilizerHolder.h"

StabilizerHolder::StabilizerHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_stabilizers.resize(libMesh::n_threads());
  _block_stabilizers.resize(libMesh::n_threads());
}

StabilizerHolder::~StabilizerHolder()
{
  {

    std::vector<std::map<unsigned int, Stabilizer *> >::iterator i;
    for (i=_active_stabilizers.begin(); i!=_active_stabilizers.end(); ++i)
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
    for (i=_block_stabilizers.begin(); i!=_block_stabilizers.end(); ++i)
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
  return _active_stabilizers[tid].begin();
}

StabilizerIterator
StabilizerHolder::activeStabilizersEnd(THREAD_ID tid)
{
  return _active_stabilizers[tid].end();
}


StabilizerIterator
StabilizerHolder::blockStabilizersBegin(THREAD_ID tid, unsigned int block_id)
{
  return _block_stabilizers[tid][block_id].begin();
}

StabilizerIterator
StabilizerHolder::blockStabilizersEnd(THREAD_ID tid, unsigned int block_id)
{
  return _block_stabilizers[tid][block_id].end();
}
