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

#ifndef DAMPERWAREHOUSE_H
#define DAMPERWAREHOUSE_H

#include "Warehouse.h"

#include <vector>

class Damper;

/**
 * Holds dampers and provides some services
 */
class DamperWarehouse : public Warehouse<Damper>
{
public:
  DamperWarehouse();
  virtual ~DamperWarehouse();

  /**
   * Adds a damper
   * @param damper Damper being added
   */
  void addDamper(MooseSharedPointer<Damper> & damper);

protected:
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<Damper> > _all_ptrs;
};

#endif // DAMPERWAREHOUSE_H
