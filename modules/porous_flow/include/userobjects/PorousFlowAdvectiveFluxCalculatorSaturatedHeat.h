//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWADVECTIEFLUXCALCULATORSATURATEDHEAT_H
#define POROUSFLOWADVECTIEFLUXCALCULATORSATURATEDHEAT_H

#include "PorousFlowAdvectiveFluxCalculatorSaturated.h"

class PorousFlowAdvectiveFluxCalculatorSaturatedHeat;

template <>
InputParameters validParams<PorousFlowAdvectiveFluxCalculatorSaturatedHeat>();

/**
 * TODO
 */
class PorousFlowAdvectiveFluxCalculatorSaturatedHeat
  : public PorousFlowAdvectiveFluxCalculatorSaturated
{
public:
  PorousFlowAdvectiveFluxCalculatorSaturatedHeat(const InputParameters & parameters);

protected:
  virtual Real getU(unsigned i) const override;

  virtual Real dU_dvar(unsigned i, unsigned pvar) const override;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> & _enthalpy;

  /// Derivative of enthalpy of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _denthalpy_dvar;
};

#endif // POROUSFLOWADVECTIEFLUXCALCULATORSATURATEDHEAT_H
