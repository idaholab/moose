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
class PINSFVMomentumPressureFlux : public FVFluxKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  PINSFVMomentumPressureFlux(const InputParameters & params);

  // Pressure term so no RC data involved
  void gatherRCData(const Elem &) override final {}
  void gatherRCData(const FaceInfo &) override final {}

protected:
  ADReal computeQpResidual() override;

  /// the porosity as a functor
  const Moose::Functor<ADReal> & _eps;
  /// The pressure
  const Moose::Functor<ADReal> & _p;
  /// which momentum component this kernel applies to
  const int _index;
};
