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

template <ComputeStage>
class ADConvectiveHeatFluxBC;

declareADValidParams(ADConvectiveHeatFluxBC);

/**
 * Boundary condition for convective heat flux where temperature and heat transfer coefficient are
 * given by material properties.
 */
template <ComputeStage compute_stage>
class ADConvectiveHeatFluxBC : public ADIntegratedBC<compute_stage>
{
public:
  static InputParameters validParams();

  ADConvectiveHeatFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Far-field temperature variable
  const ADMaterialProperty(Real) & _T_infinity;

  /// Convective heat transfer coefficient
  const ADMaterialProperty(Real) & _htc;

  /// Derivative of convective heat transfer coefficient with respect to temperature
  const ADMaterialProperty(Real) & _htc_dT;

  usingIntegratedBCMembers;
};
