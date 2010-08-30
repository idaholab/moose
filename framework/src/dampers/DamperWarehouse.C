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
