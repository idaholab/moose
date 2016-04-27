/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALCAPILLARYPRESSURELINEAR_H
#define POROUSFLOWMATERIALCAPILLARYPRESSURELINEAR_H

#include "PorousFlowMaterialCapillaryPressureBase.h"

//Forward Declarations
class PorousFlowMaterialCapillaryPressureLinear;

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureLinear>();

/**
 * Linear capillary pressure (equal to the phase saturation)
 */
class PorousFlowMaterialCapillaryPressureLinear : public PorousFlowMaterialCapillaryPressureBase
{
public:
  PorousFlowMaterialCapillaryPressureLinear(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();

  /// The maximum value of the capillary pressure
  Real _pc_max;
};

#endif //POROUSFLOWMATERIALCAPILLARYPRESSURELINEAR_H
