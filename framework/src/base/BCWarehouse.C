#include "BCWarehouse.h"

BCWarehouse::BCWarehouse()
{
}

BCWarehouse::~BCWarehouse()
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::iterator i = _bcs.begin(); i != _bcs.end(); ++i)
    for (std::vector<IntegratedBC *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;

  for (std::map<unsigned int, std::vector<NodalBC *> >::iterator i = _nodal_bcs.begin(); i != _nodal_bcs.end(); ++i)
    for (std::vector<NodalBC *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;
}

void
BCWarehouse::addBC(unsigned int boundary_id, IntegratedBC *bc)
{
  _bcs[boundary_id].push_back(bc);
}

void
BCWarehouse::addNodalBC(unsigned int boundary_id, NodalBC *bc)
{
  _nodal_bcs[boundary_id].push_back(bc);
}

std::vector<IntegratedBC *> &
BCWarehouse::getBCs(unsigned int boundary_id)
{
  return _bcs[boundary_id];
}

std::vector<NodalBC *> &
BCWarehouse::getNodalBCs(unsigned int boundary_id)
{
  return _nodal_bcs[boundary_id];
}

void
BCWarehouse::activeBoundaries(std::set<short> & set_buffer) const
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
      set_buffer.insert(curr->first);

  for (std::map<unsigned int, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
      set_buffer.insert(curr->first);
}
