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

protected:
  /**
   * Adds material to compute wall temperature from heat flux
   *
   * @param[in] flow_channel   Flow channel component
   */
  void addWallTemperatureFromHeatFluxMaterial(const Pipe & flow_channel) const;
};

#endif /* CLOSURES1PHASESIMPLE_H */
