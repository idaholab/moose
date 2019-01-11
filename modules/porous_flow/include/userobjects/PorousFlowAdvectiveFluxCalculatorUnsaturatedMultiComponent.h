//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWADVECTIEFLUXCALCULATORUNSATURATEDMULTICOMPONENT_H
#define POROUSFLOWADVECTIEFLUXCALCULATORUNSATURATEDMULTICOMPONENT_H

#include "PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent.h"

class PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent;

template <>
InputParameters validParams<PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent>();

/**
 * TODO
 */
class PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
  : public PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent
{
public:
  PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent(const InputParameters & parameters);

protected:
  virtual Real getU(unsigned i) const override;

  virtual Real dU_dvar(unsigned i, unsigned pvar) const override;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;
};

#endif // POROUSFLOWADVECTIEFLUXCALCULATORUNSATURATEDMULTICOMPONENT_H
