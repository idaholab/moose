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

// MOOSE includes
#include "WarehouseBase.h"

class Damper;

/**
 * Holds dampers and provides some services
 */
class DamperWarehouse : public WarehouseBase<Damper>
{
public:
  DamperWarehouse();
};

#endif // DAMPERWAREHOUSE_H
