/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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

/**
 * Typedef to hide implementation details
 */
typedef std::map<unsigned int, Stabilizer *>::iterator StabilizerIterator;


class StabilizerWarehouse
{
public:
  StabilizerWarehouse();
  virtual ~StabilizerWarehouse();

  Stabilizer * add(std::string stabilizer_name,
                   std::string name,
                   MooseSystem & moose_system,
                   InputParameters parameters);

  bool isStabilized(unsigned int var_num);

  StabilizerIterator activeStabilizersBegin();
  StabilizerIterator activeStabilizersEnd();

  StabilizerIterator blockStabilizersBegin(unsigned int block_id);
  StabilizerIterator blockStabilizersEnd(unsigned int block_id);

  bool activeStabilizerBlocks(std::set<subdomain_id_type> & set_buffer) const;

  void addBlockStabilizer(unsigned int block_id, unsigned int var_num, Stabilizer *stabilizer);
  void addStabilizer(unsigned int var_num, Stabilizer *stabilizer);

protected:
  std::map<unsigned int, Stabilizer *> _active_stabilizers;

  std::map<unsigned int, std::map<unsigned int, Stabilizer *> > _block_stabilizers;

  std::map<unsigned int, bool> _is_stabilized;
};

#endif // STABILIZERWAREHOUSE_H
