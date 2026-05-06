#pragma once

#include "PIMPLE.h"
#include "ReducedPressurePIMPLESolve.h"

/**
 * Executioner that reuses the stock PIMPLE executioner API, but swaps in a
 * custom PIMPLE solve object with reduced-pressure / sharp-interface hooks.
 *
 * Deriving from PIMPLE is deliberate: RhieChowMassFlux validates that the
 * executioner is a PIMPLE or SIMPLE executioner in its constructor.
 */
class ReducedPressurePIMPLE : public PIMPLE
{
public:
  static InputParameters validParams();

  ReducedPressurePIMPLE(const InputParameters & parameters);

  void init() override;
  void takeStep(Real input_dt = -1.0) override;
  Real relativeSolutionDifferenceNorm(bool check_aux) const override;

protected:
  std::set<TimeIntegrator *> getTimeIntegrators() const override;

  ReducedPressurePIMPLESolve _reduced_pimple_solve;
};
