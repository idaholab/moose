/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "StabilizerWarehouse.h"

StabilizerWarehouse::StabilizerWarehouse()
{
}

StabilizerWarehouse::~StabilizerWarehouse()
{
  for (StabilizerIterator j=_active_stabilizers.begin(); j!=_active_stabilizers.end(); ++j)
    delete j->second;

  for (std::map<subdomain_id_type, std::map<unsigned int, Stabilizer *> >::iterator j=_block_stabilizers.begin(); j!=_block_stabilizers.end(); ++j)
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
StabilizerWarehouse::blockStabilizersBegin(subdomain_id_type block_id)
{
  return _block_stabilizers[block_id].begin();
}

StabilizerIterator
StabilizerWarehouse::blockStabilizersEnd(subdomain_id_type block_id)
{
  return _block_stabilizers[block_id].end();
}

void
StabilizerWarehouse::addBlockStabilizer(subdomain_id_type block_id, unsigned int var_num, Stabilizer *stabilizer)
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
