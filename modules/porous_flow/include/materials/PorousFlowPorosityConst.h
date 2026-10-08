//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPorosityBase.h"

/**
 * Material to provide a constant value of porosity. This can be specified
 * by either a constant value in the input file, or taken from an aux variable.
 * Note: this material assumes that the porosity remains constant throughout a
 * simulation, so the coupled aux variable porosity must also remain constant.
 * An error is generated if the porosity is negative, or less than porosity_min.
 */
template <bool is_ad>
class PorousFlowPorosityConstTempl : public PorousFlowPorosityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPorosityConstTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Constant porosity (Real constant Monomial variable only so no AD version)
  const VariableValue & _input_porosity;

  /// Whether the porosity is a constant number (rather than an AuxVariable)
  const bool _porosity_is_constant;

  usingPorousFlowPorosityBaseMembers;
};

typedef PorousFlowPorosityConstTempl<false> PorousFlowPorosityConst;
typedef PorousFlowPorosityConstTempl<true> ADPorousFlowPorosityConst;
