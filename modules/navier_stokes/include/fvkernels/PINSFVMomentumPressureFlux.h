//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumPressureFlux.h"

/**
 * A flux kernel using the divergence theorem for the pressure gradient term in the momentum
 * equation
 */
class PINSFVMomentumPressureFlux : public INSFVMomentumPressureFlux
{
public:
  static InputParameters validParams();
  PINSFVMomentumPressureFlux(const InputParameters & params);

protected:
  virtual const Moose::FunctorBase<ADReal> & epsilon() const override { return _eps; }

  /// the porosity as a functor
  const Moose::Functor<ADReal> & _eps;
};
