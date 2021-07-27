//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NeumannBC.h"

/**
 * Implements a Neumann BC where D grad(u) = value * M on the boundary, where
 * value is a constant and M is a material property.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
template <bool is_ad>
class MatNeumannBCTempl : public NeumannBCTempl<is_ad>
{
public:
  static InputParameters validParams();

  MatNeumannBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Value of material property on the boundary.
  const GenericMaterialProperty<Real, is_ad> & _boundary_prop;

  using NeumannBCTempl<is_ad>::_value;

  usingGenericIntegratedBCMembers;
};

typedef MatNeumannBCTempl<false> MatNeumannBC;
typedef MatNeumannBCTempl<true> ADMatNeumannBC;
