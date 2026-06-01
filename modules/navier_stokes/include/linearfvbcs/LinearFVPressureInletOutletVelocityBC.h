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

class ElemInfo;

/**
 * Linear-FV analog of reference solver's pressureInletOutletVelocity boundary condition.
 *
 * On outflow this behaves like a zero-gradient / extrapolated velocity outlet.
 * On backflow it switches to a prescribed boundary value instead of blindly
 * extrapolating the interior velocity onto the boundary face.
 */
class LinearFVPressureInletOutletVelocityBC : public LinearFVAdvectionDiffusionBC
{
public:
  static InputParameters validParams();

  LinearFVPressureInletOutletVelocityBC(const InputParameters & parameters);

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
  Real computeOutflowBoundaryValue() const;
  Real computeOutflowBoundaryValueRHSContribution() const;
  Real computeBackflowBoundaryValue() const;

  const unsigned int _dim;
  const MooseLinearVariableFVReal * const _u_var;
  const MooseLinearVariableFVReal * const _v_var;
  const MooseLinearVariableFVReal * const _w_var;
  std::vector<const MooseLinearVariableFVReal *> _vel_vars;
  const unsigned int _index;
  const Moose::Functor<Real> & _backflow_value;
  const bool _two_term_expansion;
};
