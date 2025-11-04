//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVScalarFieldAdvection.h"

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics and that advects arbitrary scalar quantities in porous medium
 */
class PINSFVScalarFieldAdvection : public INSFVScalarFieldAdvection
{
public:
  static InputParameters validParams();
  PINSFVScalarFieldAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Local medium porosity as a functor
  const Moose::Functor<ADReal> & _eps;
};
