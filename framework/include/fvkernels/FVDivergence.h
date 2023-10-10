//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

/// FVDivergence implements a standard divergence term:
///
///     - Strong form: $\nabla \cdot \vec{v}$
///
///     - Discretized form: $\sum_f \vec{v}_f \vec{S}_f$
///
/// where $\vec{v}$ is a vector field (described by a Moose Functor)
/// which is independent of the variables.
class FVDivergence : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVDivergence(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The vector field whose divergence is added to the residual
  const Moose::Functor<ADRealVectorValue> & _vector_field;
};
