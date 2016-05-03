/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSURELINEAR_H
#define POROUSFLOWCAPILLARYPRESSURELINEAR_H

#include "PorousFlowCapillaryPressureBase.h"

//Forward Declarations
class PorousFlowCapillaryPressureLinear;

template<>
InputParameters validParams<PorousFlowCapillaryPressureLinear>();

/**
 * Linear capillary pressure (equal to the phase saturation)
 */
class PorousFlowCapillaryPressureLinear : public PorousFlowCapillaryPressureBase
{
public:
  PorousFlowCapillaryPressureLinear(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The maximum value of the capillary pressure
  const Real _pc_max;
};

#endif //POROUSFLOWCAPILLARYPRESSURELINEAR_H
