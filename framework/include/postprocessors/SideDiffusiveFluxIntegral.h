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
template <bool, T>
class SideDiffusiveFluxIntegralTempl;
typedef SideDiffusiveFluxIntegralTempl<false, Real> SideDiffusiveFluxIntegral;
typedef SideDiffusiveFluxIntegralTempl<true, Real> ADSideDiffusiveFluxIntegral;
typedef SideDiffusiveFluxIntegralTempl<false, RealVectorValue> SideVectorDiffusivityFluxIntegral;
typedef SideDiffusiveFluxIntegralTempl<true, RealVectorValue> ADVectorSideDiffusivityFluxIntegral;

template <>
InputParameters validParams<SideDiffusiveFluxIntegral>();

/**
 * This postprocessor computes a side integral of the mass flux.
 */
template <bool is_ad, T material_type>
class SideDiffusiveFluxIntegralTempl : public SideIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  SideDiffusiveFluxIntegralTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  RealVectorValue diffusivity_gradient_product<material_type>(RealVectorValue grad_u, T diffusivity);

  MaterialPropertyName _diffusivity;
  const GenericMaterialProperty<material_type, is_ad> & _diffusion_coef;
};
