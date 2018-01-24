//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
