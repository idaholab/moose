#ifndef CLOSURES1PHASEBASE_H
#define CLOSURES1PHASEBASE_H

#include "ClosuresBase.h"

class Closures1PhaseBase;

template <>
InputParameters validParams<Closures1PhaseBase>();

/**
 * Base class for 1-phase closures
 */
class Closures1PhaseBase : public ClosuresBase
{
public:
  Closures1PhaseBase(const InputParameters & params);
};

#endif /* CLOSURES1PHASEBASE_H */
