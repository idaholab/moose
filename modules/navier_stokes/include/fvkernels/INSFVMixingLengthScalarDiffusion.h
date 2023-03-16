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
#include "INSFVVelocityVariable.h"

class INSFVMixingLengthScalarDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();

  INSFVMixingLengthScalarDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// Turbulent eddy mixing length
  const Moose::Functor<ADReal> & _mixing_len;

  /// Turbulent Schmidt number (or turbulent Prandtl number)
  const Real & _schmidt_number;
};
