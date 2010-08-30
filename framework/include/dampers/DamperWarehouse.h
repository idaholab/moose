#ifndef DAMPERWAREHOUSE_H
#define DAMPERWAREHOUSE_H

#include "Damper.h"

#include <vector>
#include <map>
#include <set>

/**
 * Typedef to hide implementation details
 */
typedef std::vector<Damper *>::iterator DamperIterator;


/**
 * Holds dampers and provides some services
 */
class DamperWarehouse
{
public:
  DamperWarehouse();
  virtual ~DamperWarehouse();

  DamperIterator dampersBegin();
  DamperIterator dampersEnd();

  void addDamper(Damper *damper);

protected:
  std::vector<Damper *> _dampers;
};

#endif // DAMPERWAREHOUSE_H
