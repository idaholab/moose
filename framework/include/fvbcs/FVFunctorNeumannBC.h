//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Neumann boundary (== inflow) condition for finite volume scheme
 * where the inflow is given as a functor
 */
class FVFunctorNeumannBC : public FVFluxBC
{
public:
  FVFunctorNeumannBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _functor;
  const Moose::Functor<ADReal> & _factor;
};
