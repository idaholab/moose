#ifndef INLETSTAGNATIONENTHALPYMOMENTUM_H
#define INLETSTAGNATIONENTHALPYMOMENTUM_H

#include "FlowBoundary.h"

class InletStagnationEnthalpyMomentum;

template <>
InputParameters validParams<InletStagnationEnthalpyMomentum>();

/**
 * Boundary condition with prescribed stagnation enthalpy and momentum for flow channels
 */
class InletStagnationEnthalpyMomentum : public FlowBoundary
{
public:
  InletStagnationEnthalpyMomentum(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1Phase();
  void setup2Phase();
  void setup2PhaseNCG();
};

#endif /* INLETSTAGNATIONENTHALPYMOMENTUM_H */
