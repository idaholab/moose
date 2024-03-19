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

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics and that advects arbitrary scalar quantities
 */
class INSFVNernstTransport : public FVFluxKernel
{
public:
  static InputParameters validParams();
  INSFVNernstTransport(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

    const Moose::Functor<ADReal> & _z;
    const Moose::Functor<ADReal> & _temp;
    const Moose::Functor<ADReal> & _phi;
    const Moose::Functor<ADReal> & _coeff;

  /// Decides if a geometric arithmetic or harmonic average is used for the
  /// face interpolation of the diffusion coefficient.
  Moose::FV::InterpMethod _coeff_interp_method;
};
