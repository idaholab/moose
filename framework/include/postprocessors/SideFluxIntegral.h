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
template <bool>
class SideFluxIntegralTempl;
typedef SideFluxIntegralTempl<false> SideFluxIntegral;
typedef SideFluxIntegralTempl<true> ADSideFluxIntegral;

template <>
InputParameters validParams<SideFluxIntegral>();

/**
 * This postprocessor computes a side integral of the mass flux.
 */
template <bool is_ad>
class SideFluxIntegralTempl : public SideIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  SideFluxIntegralTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  MaterialPropertyName _diffusivity;
  const GenericMaterialProperty<Real, is_ad> & _diffusion_coef;
};
