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

  /**
   * Parameters of this object that should be added to the NSFV action that are unique to this
   * object
   */
  static InputParameters uniqueParams();

  /**
   * @returns A list of the parameters that are common between this object and the NSFV action
   */
  static std::vector<std::string> listOfCommonParams();

  void gatherRCData(const Elem &) override final {}
  void gatherRCData(const FaceInfo & fi) override final;
  void initialSetup() override;
  using INSFVAdvectionKernel::computeResidual;
  void computeResidual(const FaceInfo & fi) override final;
  using INSFVAdvectionKernel::computeJacobian;
  void computeJacobian(const FaceInfo & fi) override final;

protected:
  virtual ADReal computeQpResidual() override final;
  virtual bool hasMaterialTimeDerivative() const override { return true; }

  /**
   * A virtual method that allows us to reuse all the code from free-flow for porous
   */
  virtual const Moose::FunctorBase<ADReal> & epsilon() const { return _unity_functor; }

  /**
   * Helper method that computes the 'a' coefficients and AD residuals
   */
  virtual void computeResidualsAndAData(const FaceInfo & fi);

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Whether to approximately calculate the 'a' coefficients
  const bool _approximate_as;

  /// Characteristic speed
  const Real _cs;

  // Attributes to compute the momentum limiters based on the momentum absolute value
  // This is desirable to make the solution independent of the orientation of the mesh
  /// Problem dimension
  const unsigned int _dim;
  /// Velocity X-component
  const Moose::Functor<ADReal> & _u;
  /// Velocity Y-component
  const Moose::Functor<ADReal> & _v;
  /// Velocity Z-component
  const Moose::Functor<ADReal> & _w;
  /// Whether to use a vector limiter
  const bool _use_norm_for_momentum_limiter;
  /// Momentum norm
  std::unique_ptr<PiecewiseByBlockLambdaFunctor<ADReal>> _mom_abs;

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
