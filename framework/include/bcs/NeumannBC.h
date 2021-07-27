//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericIntegratedBC.h"

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
template <bool is_ad>
class NeumannBCTempl : public GenericIntegratedBC<is_ad>
{
public:
  static InputParameters validParams();

  NeumannBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Value of grad(u) on the boundary.
  const Real & _value;

  usingGenericIntegratedBCMembers;
};

typedef NeumannBCTempl<false> NeumannBC;
typedef NeumannBCTempl<true> ADNeumannBC;
