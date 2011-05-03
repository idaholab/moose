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
#include "MooseVariable.h"

StabilizerWarehouse::StabilizerWarehouse()
{
}

StabilizerWarehouse::~StabilizerWarehouse()
{
  for (std::vector<Stabilizer *>::const_iterator j = _active_stabilizers.begin(); j != _active_stabilizers.end(); ++j)
    delete *j;

  for (std::map<unsigned int, std::vector<Stabilizer *> >::iterator j = _block_stabilizers.begin(); j != _block_stabilizers.end(); ++j)
    for(std::vector<Stabilizer *>::iterator k = (j->second).begin(); k != (j->second).end(); ++k)
      delete *k;
}

bool
StabilizerWarehouse::isStabilized(unsigned int var_num)
{
  return _is_stabilized[var_num];
}

void
StabilizerWarehouse::addStabilizer(Stabilizer *stabilizer)
{
  unsigned int var_num = stabilizer->variable().number();
  _active_stabilizers.push_back(stabilizer);
  _is_stabilized[var_num] = true;
}

void
StabilizerWarehouse::addBlockStabilizer(unsigned int block_id, Stabilizer *stabilizer)
{
  unsigned int var_num = stabilizer->variable().number();
  _block_stabilizers[block_id].push_back(stabilizer);
  _is_stabilized[var_num] = true;
}
