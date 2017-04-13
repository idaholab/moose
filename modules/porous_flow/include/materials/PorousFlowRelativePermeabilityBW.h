/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYBW_H
#define POROUSFLOWRELATIVEPERMEABILITYBW_H

#include "PorousFlowRelativePermeabilityBase.h"
#include "PorousFlowBroadbridgeWhite.h"

class PorousFlowRelativePermeabilityBW;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityBW>();

/**
 * Material that calculates the Broadbridge-White relative permeability
 * P Broadbridge, I White ``Constant rate rainfall
 * infiltration: A versatile nonlinear model, 1 Analytical solution''.
 * Water Resources Research 24 (1988) 145--154.
 */
class PorousFlowRelativePermeabilityBW : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityBW(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// BW's low saturation
  const Real _sn;

  /// BW's high saturation
  const Real _ss;

  /// BW's low relative permeability
  const Real _kn;

  /// BW's high relative permeability
  const Real _ks;

  /// BW's C parameter
  const Real _c;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYBW_H
