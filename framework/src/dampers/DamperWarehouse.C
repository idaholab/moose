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

#include "DamperWarehouse.h"

#include "MooseSystem.h"
#include "Damper.h"

DamperWarehouse::DamperWarehouse()
{
}

DamperWarehouse::~DamperWarehouse()
{
  DamperIterator j;
  for (j=_dampers.begin(); j!=_dampers.end(); ++j)
    delete *j;
}

void
DamperWarehouse::addDamper(Damper *damper)
{
  _dampers.push_back(damper);
}

DamperIterator
DamperWarehouse::dampersBegin()
{
  return _dampers.begin();
}

DamperIterator
DamperWarehouse::dampersEnd()
{
  return _dampers.end();
}
