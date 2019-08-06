#pragma once

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

protected:
  virtual void check() const override;
};
