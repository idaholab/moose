//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"
#include "NS.h"

class ElemInfo;

/**
 * Conservative rho*u variant of the pressureInletOutletVelocity boundary condition.
 *
 * User-facing backflow values are still specified in velocity units, but the solved variable and
 * the returned boundary values are rho*u.
 */
class LinearFVPressureInletOutletMomentumBC : public LinearFVAdvectionDiffusionBC
{
public:
  static InputParameters validParams();

  LinearFVPressureInletOutletMomentumBC(const InputParameters & parameters);

  Real computeBoundaryValue() const override;
  Real computeBoundaryNormalGradient() const override;
  Real computeBoundaryValueMatrixContribution() const override;
  Real computeBoundaryValueRHSContribution() const override;
  Real computeBoundaryGradientMatrixContribution() const override;
  Real computeBoundaryGradientRHSContribution() const override;

  bool includesMaterialPropertyMultiplier() const override { return !isBackflow(); }
  bool useBoundaryGradientExtrapolation() const override { return isBackflow(); }

protected:
  bool isBackflow() const;
  const ElemInfo & fluidElemInfo() const;
  Real safeDensity(Real rho) const;
  Real fluidDensity(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  Real computeOutflowBoundaryValue() const;
  Real computeOutflowBoundaryValueRHSContribution() const;
  Real computeBackflowBoundaryValue() const;
  Real computeBackflowBoundaryValueMatrixContribution() const;
  Real computeVelocity(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  RealGradient computeVelocityGradient(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  Real boundaryDensity() const;

  const unsigned int _dim;
  const MooseLinearVariableFVReal * const _u_var;
  const MooseLinearVariableFVReal * const _v_var;
  const MooseLinearVariableFVReal * const _w_var;
  std::vector<const MooseLinearVariableFVReal *> _momentum_vars;
  const unsigned int _index;
  const Moose::Functor<Real> & _rho;
  const Moose::Functor<RealVectorValue> * const _density_gradient;
  const Moose::Functor<Real> & _backflow_value;
  const Real _minimum_density;
  const bool _two_term_expansion;
};
