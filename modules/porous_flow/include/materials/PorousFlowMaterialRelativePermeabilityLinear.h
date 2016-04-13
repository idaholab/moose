/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALRELATIVEPERMEABILITYLINEAR_H
#define POROUSFLOWMATERIALRELATIVEPERMEABILITYLINEAR_H

#include "PorousFlowMaterialRelativePermeabilityBase.h"

//Forward Declarations
class PorousFlowMaterialRelativePermeabilityLinear;

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityLinear>();

/**
 * Linear relative permeability (equal to the phase saturation)
 */
class PorousFlowMaterialRelativePermeabilityLinear : public PorousFlowMaterialRelativePermeabilityBase
{
public:
  PorousFlowMaterialRelativePermeabilityLinear(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();
};

#endif //POROUSFLOWMATERIALRELATIVEPERMEABILITYLINEAR_H
