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

/*
 *  FVCoupledAdvection implements a standard advection term
 *  where the advection component is the gradent of a coupled variable:
 *
 *      - strong form: \nabla  u \nabla v
 *
 *      - weak form: \int_{A} u \nabla v \cdot \vec{n} dA
 */
class FVCoupledAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVCoupledAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

private:
  /// The variable data of the couple variable
  const MooseVariableFieldBase & _v_var;
};
