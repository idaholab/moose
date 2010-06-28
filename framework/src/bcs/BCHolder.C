#include "BCHolder.h"

BCHolder::BCHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_bcs.resize(libMesh::n_threads());
  _active_nodal_bcs.resize(libMesh::n_threads());
}

BCHolder::~BCHolder()
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
BCHolder::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _active_bcs.resize(n_threads);
  _active_nodal_bcs.resize(n_threads);
}

void
BCHolder::addBC(THREAD_ID tid, unsigned int boundary_id, BoundaryCondition *bc)
{
  _active_bcs[tid][boundary_id].push_back(bc);
}

void
BCHolder::addNodalBC(THREAD_ID tid, unsigned int boundary_id, BoundaryCondition *bc)
{
  _active_nodal_bcs[tid][boundary_id].push_back(bc);
}

BCIterator
BCHolder::activeBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].begin();
}

BCIterator
BCHolder::activeBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].end();
}


BCIterator
BCHolder::activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_nodal_bcs[tid][boundary_id].begin();
}

BCIterator
BCHolder::activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_nodal_bcs[tid][boundary_id].end();
}

void
BCHolder::activeBoundaries(std::set<subdomain_id_type> & set_buffer) const
{
  std::map<unsigned int, std::vector<BoundaryCondition *> >::const_iterator curr, end;
  end = _active_bcs[0].end();

  try
  {
    for (curr = _active_bcs[0].begin(); curr != end; ++curr)
    {
      set_buffer.insert(subdomain_id_type(curr->first));
    }
  }
  catch (std::exception & /*e*/)
  {
    mooseError("Invalid block specified in input file");
  }
}
