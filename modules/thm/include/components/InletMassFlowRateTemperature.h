#ifndef INLETMASSFLOWRATETEMPERATURE_H
#define INLETMASSFLOWRATETEMPERATURE_H

#include "FlowBoundary.h"

class InletMassFlowRateTemperature;

template <>
InputParameters validParams<InletMassFlowRateTemperature>();

/**
 * Boundary condition with prescribed mass flow rate and temperature for flow channels
 *
 */
class InletMassFlowRateTemperature : public FlowBoundary
{
public:
  InletMassFlowRateTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1PhaseCG();
  void setup1PhaseRDG();
  void setup2PhaseCG();
  void setup2PhaseRDG();
  void setup2PhaseNCGCG();
};

#endif /* INLETMASSFLOWRATETEMPERATURE_H */
