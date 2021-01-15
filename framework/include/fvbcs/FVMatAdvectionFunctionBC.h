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

class FVMatAdvectionFunctionBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  FVMatAdvectionFunctionBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _vel;

  /// The advected quantity on the elem
  const MooseArray<ADReal> & _adv_quant;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  const Function * const _flux_variable_exact_solution;
  const Function & _vel_x_exact_solution;
  const Function * const _vel_y_exact_solution;
  const Function * const _vel_z_exact_solution;
};
