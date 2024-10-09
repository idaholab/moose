//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Boundary condition for convective heat flux where temperature and heat transfer coefficient are
 * given by material properties.
 */
class ADConvectiveHeatFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  ADConvectiveHeatFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Far-field temperature variable
  const ADMaterialProperty<Real> * const _T_infinity;

  /// Convective heat transfer coefficient
  const ADMaterialProperty<Real> * const _htc;

  /// Far-field temperature functor
  const Moose::Functor<ADReal> * const _T_infinity_functor;

  /// Convective heat transfer coefficient as a functor
  const Moose::Functor<ADReal> * const _htc_functor;
};
