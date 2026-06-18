//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

protected:
  PIMPLESolve & pimpleSolve() override { return _reduced_pimple_solve; }
  const PIMPLESolve & pimpleSolve() const override { return _reduced_pimple_solve; }
  void postTakeStep() override;

  ReducedPressurePIMPLESolve _reduced_pimple_solve;
};
