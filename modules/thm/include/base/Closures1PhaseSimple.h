#ifndef CLOSURES1PHASESIMPLE_H
#define CLOSURES1PHASESIMPLE_H

#include "Closures1PhaseBase.h"

class Closures1PhaseSimple;

template <>
InputParameters validParams<Closures1PhaseSimple>();

/**
 * Simple 1-phase closures
 */
class Closures1PhaseSimple : public Closures1PhaseBase
{
public:
  Closures1PhaseSimple(const InputParameters & params);

  virtual void check(const Pipe & flow_channel) const override;
  virtual void addMooseObjects(const Pipe & flow_channel) override;
};

#endif /* CLOSURES1PHASESIMPLE_H */
