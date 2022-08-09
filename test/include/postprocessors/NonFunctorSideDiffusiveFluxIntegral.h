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
#include "SideIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

template <bool is_ad>
class NonFunctorSideDiffusiveFluxIntegralTempl : public SideIntegralPostprocessor,
                                                 public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NonFunctorSideDiffusiveFluxIntegralTempl(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;

  const VariableValue & _u;
  const VariableGradient & _grad_u;
  const bool _fv;

  const GenericMaterialProperty<Real, is_ad> & _diffusion_coef;

private:
  /// Routine to get the diffusive flux with a Real diffusivity
  RealVectorValue diffusivityGradientProduct(const RealVectorValue & grad_u, Real diffusivity);
};

typedef NonFunctorSideDiffusiveFluxIntegralTempl<false> NonFunctorSideDiffusiveFluxIntegral;
typedef NonFunctorSideDiffusiveFluxIntegralTempl<true> ADNonFunctorSideDiffusiveFluxIntegral;
