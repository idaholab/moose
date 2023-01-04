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
 * Robin boundary condition (temperatures) for finite volume scheme between
 * a solid and fluid where the temperatures and heat transfer coefficient
 * are given as a functors
 */
class FVFunctorConvectiveHeatFluxBC : public FVFluxBC
{
public:
  FVFunctorConvectiveHeatFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// Wall temperature functor
  const Moose::Functor<ADReal> & _T_solid;

  /// Far-field temperature functor
  const Moose::Functor<ADReal> & _T_bulk;

  /// Convective heat transfer coefficient functor
  const Moose::Functor<ADReal> & _htc;

  /// Boolean specifying if domain is solid or fluid
  const bool _is_solid;
};
