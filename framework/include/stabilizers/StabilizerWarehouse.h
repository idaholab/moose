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

#ifndef STABILIZERWAREHOUSE_H
#define STABILIZERWAREHOUSE_H

#include "Stabilizer.h"

class StabilizerWarehouse
{
public:
  StabilizerWarehouse();
  virtual ~StabilizerWarehouse();

  bool isStabilized(unsigned int var_num);

  const std::vector<Stabilizer *> & active() { return _active_stabilizers; }

  const std::vector<Stabilizer *> & blockStabilizers(unsigned int block_id) { return _block_stabilizers[block_id]; }

  void addStabilizer(Stabilizer *stabilizer);

  void addBlockStabilizer(unsigned int block_id, Stabilizer *stabilizer);

protected:
  std::vector<Stabilizer *> _active_stabilizers;

  std::map<unsigned int, std::vector<Stabilizer *> > _block_stabilizers;

  std::map<unsigned int, bool> _is_stabilized;
};

#endif // STABILIZERWAREHOUSE_H
