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

#include "DamperWarehouse.h"
#include "Damper.h"

DamperWarehouse::DamperWarehouse() :
    Warehouse<Damper>()
{
}

DamperWarehouse::~DamperWarehouse()
{
}

void
DamperWarehouse::addDamper(MooseSharedPointer<Damper> & damper)
{
  _all_ptrs.push_back(damper);
  _all_objects.push_back(damper.get());
}
