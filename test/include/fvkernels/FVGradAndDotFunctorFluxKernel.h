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

class FVGradAndDotFunctorFluxKernel : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVGradAndDotFunctorFluxKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _value;
  const Moose::Functor<ADRealVectorValue> & _gradient;
  const Moose::Functor<ADReal> & _dot;
  const RealVectorValue _velocity;
};
