//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "CompileTimeDerivativesMaterial.h"

/**
 * Material class that creates the math free energy with the compile time derivatives framework.
 * \f$ F = \frac14 (1 + c)^2 (1 - c)^2 \f$.
 */
template <bool is_ad>
class MathCTDFreeEnergyTempl : public CompileTimeDerivativesMaterial<1, is_ad, 2>
{
public:
  static InputParameters validParams();

  MathCTDFreeEnergyTempl(const InputParameters & parameters);

protected:
  void computeQpProperties();

  using CompileTimeDerivativesMaterial<1, is_ad, 2>::evaluate;
  using CompileTimeDerivativesMaterial<1, is_ad, 2>::_refs;
};

typedef MathCTDFreeEnergyTempl<false> MathCTDFreeEnergy;
typedef MathCTDFreeEnergyTempl<true> ADMathCTDFreeEnergy;
