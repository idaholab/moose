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
#include "INSFVMomentumResidualObject.h"

/**
 * A flux kernel using the divergence theorem for the pressure gradient term in the momentum
 * equation
 */
class INSFVMomentumPressureFlux : public FVFluxKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVMomentumPressureFlux(const InputParameters & params);

  // Pressure term so no RC data involved
  void gatherRCData(const Elem &) override final {}
  void gatherRCData(const FaceInfo &) override final {}

protected:
  virtual ADReal computeQpResidual() override;
  virtual const Moose::FunctorBase<ADReal> & epsilon() const { return _unity_functor; }

  /// The pressure
  const Moose::Functor<ADReal> & _p;

  /// A unity functor used in the epsilon virtual method
  const Moose::ConstantFunctor<ADReal> _unity_functor{1};
};
