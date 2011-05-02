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

DamperWarehouse::DamperWarehouse()
{
}

DamperWarehouse::~DamperWarehouse()
{
  for (std::vector<Damper *>::const_iterator j = _dampers.begin(); j != _dampers.end(); ++j)
    delete *j;
}

void
DamperWarehouse::addDamper(Damper *damper)
{
  _dampers.push_back(damper);
}
