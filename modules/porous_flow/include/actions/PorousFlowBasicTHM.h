//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
