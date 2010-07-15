#include "BCWarehouse.h"

BCWarehouse::BCWarehouse(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_bcs.resize(libMesh::n_threads());
  _active_nodal_bcs.resize(libMesh::n_threads());
}

BCWarehouse::~BCWarehouse()
{
  {
    std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > >::iterator i;
    for(i=_active_bcs.begin(); i!=_active_bcs.end(); ++i)
    {
      std::map<unsigned int, std::vector<BoundaryCondition *> >::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        BCIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > >::iterator i;
    for(i=_active_nodal_bcs.begin(); i!=_active_nodal_bcs.end(); ++i)
    {
      std::map<unsigned int, std::vector<BoundaryCondition *> >::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        BCIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }
}

void
BCWarehouse::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _active_bcs.resize(n_threads);
  _active_nodal_bcs.resize(n_threads);
}

void
BCWarehouse::addBC(THREAD_ID tid, unsigned int boundary_id, BoundaryCondition *bc)
{
  _active_bcs[tid][boundary_id].push_back(bc);
}

void
BCWarehouse::addNodalBC(THREAD_ID tid, unsigned int boundary_id, BoundaryCondition *bc)
{
  _active_nodal_bcs[tid][boundary_id].push_back(bc);
}

BCIterator
BCWarehouse::activeBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].begin();
}

BCIterator
BCWarehouse::activeBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].end();
}


BCIterator
BCWarehouse::activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_nodal_bcs[tid][boundary_id].begin();
}

BCIterator
BCWarehouse::activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_nodal_bcs[tid][boundary_id].end();
}

void
BCWarehouse::activeBoundaries(std::set<short> & set_buffer) const
{
  std::map<unsigned int, std::vector<BoundaryCondition *> >::const_iterator curr;

  for (curr = _active_bcs[0].begin(); curr != _active_bcs[0].end(); ++curr)
      set_buffer.insert(curr->first);

  for (curr = _active_nodal_bcs[0].begin(); curr != _active_nodal_bcs[0].end(); ++curr)
      set_buffer.insert(curr->first);
}
