/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POROUSFLOWBASICTHM_H
#define POROUSFLOWBASICTHM_H

#include "PorousFlowSinglePhaseBase.h"

class PorousFlowBasicTHM;

template <>
InputParameters validParams<PorousFlowBasicTHM>();

/**
 * Action for simulation involving a single phase, single component,
 * fully saturated fluid, using no upwinding, no mass lumping of the
 * fluid mass, linearised fluid-mass time derivative, and potentially
 * no multiplication by density of the fluid kernels
 */
class PorousFlowBasicTHM : public PorousFlowSinglePhaseBase
{
public:
  PorousFlowBasicTHM(const InputParameters & params);

  virtual void act() override;

protected:
  // whether to multiply the fluid kernels by the fluid density
  const bool _multiply_by_density;
};

#endif // POROUSFLOWBASICTHM_H
