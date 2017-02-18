/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POROUSFLOWFULLYSATURATED_H
#define POROUSFLOWFULLYSATURATED_H

#include "PorousFlowSinglePhaseBase.h"

class PorousFlowFullySaturated;

template <>
InputParameters validParams<PorousFlowFullySaturated>();

/**
 * Action for simulation involving a single phase fully saturated fluid.
 */
class PorousFlowFullySaturated : public PorousFlowSinglePhaseBase
{
public:
  PorousFlowFullySaturated(const InputParameters & params);

  virtual void act() override;
};

#endif // POROUSFLOWFULLYSATURATED_H
