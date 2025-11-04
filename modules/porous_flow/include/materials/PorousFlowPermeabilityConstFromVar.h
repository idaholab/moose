//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityBase.h"

/**
 * Material to provide permeability taken from a variable. This material
 * is primarily designed for use with heterogeneous reservoir models
 * where the components of the permeability tensor are provided by an
 * elemental aux variables that do not change.
 * The three diagonal entries corresponding to the x, y, and z directions
 * must be given. Optionally, the off-diagonal components of the full
 * permeability tensor can be given. If they are not provided, they will be
 * initialised to zero.
 */
template <bool is_ad>
class PorousFlowPermeabilityConstFromVarTempl : public PorousFlowPermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityConstFromVarTempl(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Permeability components
  /// Note: these can only be constant (Real constant Monomial auxvariables) so no AD version
  const VariableValue & _perm_xx;
  const VariableValue & _perm_xy;
  const VariableValue & _perm_xz;
  const VariableValue & _perm_yx;
  const VariableValue & _perm_yy;
  const VariableValue & _perm_yz;
  const VariableValue & _perm_zx;
  const VariableValue & _perm_zy;
  const VariableValue & _perm_zz;

  usingPorousFlowPermeabilityBaseMembers;
};

typedef PorousFlowPermeabilityConstFromVarTempl<false> PorousFlowPermeabilityConstFromVar;
typedef PorousFlowPermeabilityConstFromVarTempl<true> ADPorousFlowPermeabilityConstFromVar;
