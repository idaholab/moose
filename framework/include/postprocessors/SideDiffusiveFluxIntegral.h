//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SideIntegralVariablePostprocessor.h"

// Forward Declarations
template <bool, typename>
class SideDiffusiveFluxIntegralTempl;
typedef SideDiffusiveFluxIntegralTempl<false, Real> SideDiffusiveFluxIntegral;
typedef SideDiffusiveFluxIntegralTempl<true, Real> ADSideDiffusiveFluxIntegral;
typedef SideDiffusiveFluxIntegralTempl<false, RealVectorValue> SideVectorDiffusivityFluxIntegral;
typedef SideDiffusiveFluxIntegralTempl<true, RealVectorValue> ADSideVectorDiffusivityFluxIntegral;

template <>
InputParameters validParams<SideDiffusiveFluxIntegral>();

/**
 * This postprocessor computes a side integral of the mass flux.
 */
template <bool is_ad, typename T>
class SideDiffusiveFluxIntegralTempl : public SideIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  SideDiffusiveFluxIntegralTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  MaterialPropertyName _diffusivity;
  const GenericMaterialProperty<T, is_ad> & _diffusion_coef;

private:
  /// Routine to get the diffusive flux with a Real diffusivity
  RealVectorValue diffusivity_gradient_product(RealVectorValue grad_u, Real diffusivity);

  /// Routine to get the diffusive flux with a RealVectorValue diffusivity
  RealVectorValue diffusivity_gradient_product(RealVectorValue grad_u, RealVectorValue diffusivity);
};
