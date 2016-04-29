/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALCAPILLARYPRESSURECONSTANT_H
#define POROUSFLOWMATERIALCAPILLARYPRESSURECONSTANT_H

#include "PorousFlowMaterialCapillaryPressureBase.h"

class PorousFlowMaterialCapillaryPressureConstant;

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureConstant>();

/**
 * Returns a constant capillary pressure as specified in the input file
 */
class PorousFlowMaterialCapillaryPressureConstant : public PorousFlowMaterialCapillaryPressureBase
{
public:
  PorousFlowMaterialCapillaryPressureConstant(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The capillary pressure (Pa)
  Real _pc;
};

#endif //POROUSFLOWMATERIALCAPILLARYPRESSURECONSTANT_H
