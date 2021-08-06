//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumAdvectionOutflowBC.h"

/**
 * A class for finite volume fully developed outflow boundary conditions for the momentum equation
 * It advects superficial momentum at the outflow, and may replace outlet pressure boundary
 * conditions when selecting a mean-pressure approach.
 */
class PINSFVMomentumAdvectionOutflowBC : public INSFVMomentumAdvectionOutflowBC
{
public:
  static InputParameters validParams();
  PINSFVMomentumAdvectionOutflowBC(const InputParameters & params);

protected:
  const Moose::FunctorBase<ADReal> & epsFunctor() const override { return _eps; }

  /// porosity
  const Moose::Functor<ADReal> & _eps;
};
