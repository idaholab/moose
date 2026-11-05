//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVAdvectionKernel.h"

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics and that advects arbitrary scalar quantities
 */
class INSFVScalarFieldAdvection : public INSFVAdvectionKernel
{
public:
  static InputParameters validParams();
  INSFVScalarFieldAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  virtual bool hasMaterialTimeDerivative() const override { return true; }

  /// The dimension of the simulation
  const unsigned int _dim;

  /// x component of the relative velocity added to the mixture velocity
  const Moose::Functor<ADReal> * const _u_slip;
  /// y component of the relative velocity added to the mixture velocity
  const Moose::Functor<ADReal> * const _v_slip;
  /// z component of the relative velocity added to the mixture velocity
  const Moose::Functor<ADReal> * const _w_slip;

  /// Whether a relative velocity was supplied
  bool _add_slip_model;
};
