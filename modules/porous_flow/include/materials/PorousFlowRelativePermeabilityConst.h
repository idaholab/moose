/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWRELATIVEPERMEABILITYCONST_H
#define POROUSFLOWRELATIVEPERMEABILITYCONST_H

#include "PorousFlowRelativePermeabilityBase.h"

class PorousFlowRelativePermeabilityConst;

template <>
InputParameters validParams<PorousFlowRelativePermeabilityConst>();

/**
 * This class simply sets a constant relative permeability at the nodes. This
 * simple class is useful for testing purposes mainly
 */
class PorousFlowRelativePermeabilityConst : public PorousFlowRelativePermeabilityBase
{
public:
  PorousFlowRelativePermeabilityConst(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Constant relative permeability
  const Real _relperm;
};

#endif // POROUSFLOWRELATIVEPERMEABILITYCONST_H
