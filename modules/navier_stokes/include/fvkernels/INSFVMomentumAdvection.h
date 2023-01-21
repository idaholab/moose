//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVAdvectionKernel.h"
#include "INSFVMomentumResidualObject.h"
#include "PiecewiseByBlockLambdaFunctor.h"

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class INSFVMomentumAdvection : public INSFVAdvectionKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVMomentumAdvection(const InputParameters & params);
  void gatherRCData(const Elem &) override final {}
  void gatherRCData(const FaceInfo & fi) override final;
  void initialSetup() override;
  using INSFVAdvectionKernel::computeResidual;
  void computeResidual(const FaceInfo & fi) override;
  using INSFVAdvectionKernel::computeJacobian;
  void computeJacobian(const FaceInfo & fi) override;

protected:
  ADReal computeQpResidual() override;

  /**
   * A virtual method that allows us to reuse all the code from free-flow for porous
   */
  virtual const Moose::FunctorBase<ADReal> & epsilon() const { return _unity_functor; }

  /**
   * Helper method that computes the 'a' coefficients and AD residuals
   */
  void computeResidualsAndAData(const FaceInfo & fi);

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Our local momentum functor
  std::unique_ptr<PiecewiseByBlockLambdaFunctor<ADReal>> _rho_u;

  /// The a coefficient for the element
  ADReal _ae = 0;

  /// The a coefficient for the neighbor
  ADReal _an = 0;

  /// The element residual
  ADReal _elem_residual = 0;

  /// The neighbor residual
  ADReal _neighbor_residual = 0;

  /// A unity functor used in the epsilon virtual method
  const Moose::ConstantFunctor<ADReal> _unity_functor{1};
};
