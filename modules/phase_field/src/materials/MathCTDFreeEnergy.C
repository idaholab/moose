//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathCTDFreeEnergy.h"

registerMooseObject("PhaseFieldApp", MathCTDFreeEnergy);
registerMooseObject("PhaseFieldApp", ADMathCTDFreeEnergy);

template <bool is_ad>
InputParameters
MathCTDFreeEnergyTempl<is_ad>::validParams()
{
  InputParameters params = CompileTimeDerivativesMaterial<1, is_ad, 2>::validParams();
  params.addClassDescription("Material that implements the math free energy using the expression "
                             "builder and automatic differentiation");
  params.addRequiredCoupledVar("c", "Concentration variable");
  return params;
}

template <bool is_ad>
MathCTDFreeEnergyTempl<is_ad>::MathCTDFreeEnergyTempl(const InputParameters & parameters)
  : CompileTimeDerivativesMaterial<1, is_ad, 2>(parameters, {"c"})
{
}

template <bool is_ad>
void
MathCTDFreeEnergyTempl<is_ad>::computeQpProperties()
{
  const auto & [c] = _refs;
  const auto F = 1.0 / 4.0 * (1.0 + c) * (1.0 + c) * (1.0 - c) * (1.0 - c);

  evaluate(F);
}
