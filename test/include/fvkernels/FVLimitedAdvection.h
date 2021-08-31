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

#include <memory>

namespace Moose
{
namespace FV
{
template <typename>
class Limiter;
}
}

/**
 * Implements an advection term in which a slope limiter is applied when interpolating the advected
 * quantity to the face
 */
class FVLimitedAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVLimitedAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const RealVectorValue _velocity;

  std::unique_ptr<Moose::FV::Limiter<ADReal>> _limiter;

  const ADVariableGradient & _grad_u_elem;
  const ADVariableGradient & _grad_u_neighbor;
};
