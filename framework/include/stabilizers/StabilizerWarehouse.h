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

/**
 * Stores stabilizers
 */
class StabilizerWarehouse
{
public:
  StabilizerWarehouse();
  virtual ~StabilizerWarehouse();

  /**
   * Is a variable stabilized
   * @param var_num The number of variable
   * @return true if the variable is stabilized, otherwise false
   */
  bool isStabilized(unsigned int var_num);

  /**
   * Gets the list of active stabilizers
   * @return The list of stabilizers
   */
  const std::vector<Stabilizer *> & active() { return _active_stabilizers; }

  /**
   * Get the list of stabilizer for a given block
   * @param block_id ID of the block we are querying
   * @return The list of stabilizers on the block_id
   */
  const std::vector<Stabilizer *> & blockStabilizers(unsigned int block_id) { return _block_stabilizers[block_id]; }

  /**
   * Add a stabilizer
   * @param stabilizer The stabilizer being added
   */
  void addStabilizer(Stabilizer *stabilizer);

  /**
   * Add a stabilizer that works only on a block
   * @param block_id ID of the block this stabilizer works on
   * @param stabilizer The stabilizer being added
   */
  void addBlockStabilizer(unsigned int block_id, Stabilizer *stabilizer);

protected:
  std::vector<Stabilizer *> _active_stabilizers;                                ///< list of stabilizers (global)

  std::map<unsigned int, std::vector<Stabilizer *> > _block_stabilizers;        ///< stabilizers on blocks

  std::map<unsigned int, bool> _is_stabilized;                                  ///< stabilization info (variable -> stabilized or not)
};

#endif // STABILIZERWAREHOUSE_H
