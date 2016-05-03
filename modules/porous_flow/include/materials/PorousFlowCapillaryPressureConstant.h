/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSURECONSTANT_H
#define POROUSFLOWCAPILLARYPRESSURECONSTANT_H

#include "PorousFlowCapillaryPressureBase.h"

class PorousFlowCapillaryPressureConstant;

template<>
InputParameters validParams<PorousFlowCapillaryPressureConstant>();

/**
 * Returns a constant capillary pressure as specified in the input file
 */
class PorousFlowCapillaryPressureConstant : public PorousFlowCapillaryPressureBase
{
public:
  PorousFlowCapillaryPressureConstant(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The capillary pressure (Pa)
  const Real _pc;
};

#endif //POROUSFLOWCAPILLARYPRESSURECONSTANT_H
