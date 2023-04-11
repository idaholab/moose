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
#include "Limiter.h"
#include "VectorCompositeFunctor.h"

/**
 * Implements an advection term in which a slope limiter is applied when interpolating the advected
 * quantity to the face
 */
class FVLimitedVectorAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVLimitedVectorAdvection(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const RealVectorValue _velocity;

  const Moose::FV::LimiterType _limiter_type;
  const Moose::VectorCompositeFunctor<ADReal> _vector;
  const unsigned int _index;
};
