/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALRELATIVEPERMEABILITYCONSTANT_H
#define POROUSFLOWMATERIALRELATIVEPERMEABILITYCONSTANT_H

#include "PorousFlowMaterialRelativePermeabilityBase.h"

class PorousFlowMaterialRelativePermeabilityConstant;

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityConstant>();

/**
 * Returns a constant relative permeability (1.0)
 */
class PorousFlowMaterialRelativePermeabilityConstant : public PorousFlowMaterialRelativePermeabilityBase
{
public:
  PorousFlowMaterialRelativePermeabilityConstant(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();
};

#endif //POROUSFLOWMATERIALRELATIVEPERMEABILITYCONSTANT_H
