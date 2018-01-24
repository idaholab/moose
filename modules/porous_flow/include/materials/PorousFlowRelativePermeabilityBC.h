/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYBC_H
#define POROUSFLOWRELATIVEPERMEABILITYBC_H

#include "PorousFlowRelativePermeabilityBase.h"

class PorousFlowRelativePermeabilityBC;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityBC>();

/**
 * Material to calculate Brooks-Corey relative permeability of an arbitrary phase
 * given the effective saturation and exponent of that phase.
 *
 * From Brooks, R. H. and A. T. Corey (1966), Properties of porous media affecting
 * fluid flow, J. Irrig. Drain. Div., 6, 61
 */
class PorousFlowRelativePermeabilityBC : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityBC(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Brooks-Corey exponent lambda
  const Real _lambda;
  /// Flag that is set to true if this is the non-wetting (gas) phase
  const bool _is_nonwetting;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYBC_H
