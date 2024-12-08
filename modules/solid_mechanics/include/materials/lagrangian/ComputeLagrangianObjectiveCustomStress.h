//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianObjectiveStress.h"

#include "MandelConverter.h"

/**
 * Wrapper for custom (external) material models that compute small stress (and small Jacobian)
 *
 * This class performs the objective integration for the custom (external) material model.
 */
template <bool symmetric>
class ComputeLagrangianObjectiveCustomStressTmpl : public ComputeLagrangianObjectiveStress
{
public:
  static InputParameters validParams();

  ComputeLagrangianObjectiveCustomStressTmpl(const InputParameters & parameters);

  using StressType = typename MandelConversion<RankTwoTensor, symmetric>::to;
  using JacobianType = typename MandelConversion<RankFourTensor, symmetric>::to;

protected:
  virtual void computeQpSmallStress();

protected:
  const MaterialProperty<StressType> & _custom_stress;
  const MaterialProperty<JacobianType> & _custom_jacobian;
};

using ComputeLagrangianObjectiveCustomStress = ComputeLagrangianObjectiveCustomStressTmpl<false>;
using ComputeLagrangianObjectiveCustomSymmetricStress =
    ComputeLagrangianObjectiveCustomStressTmpl<true>;
