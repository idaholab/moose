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
    MooseObjectWarehouse<Damper>()
{
}

const MooseObjectStorage<Damper> &
DamperWarehouse::getStorage() const
{
  return _all_objects;
}
