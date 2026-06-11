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
 * Pressure-controlled inlet/outlet velocity boundary condition for the sharp-interface path.
 *
 * The historical class name is retained, but the solved variable and returned boundary values are
 * velocity components, not rho*u.
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
  Real outwardFaceFlux() const;
  const ElemInfo & fluidElemInfo() const;
  Real computeOutflowBoundaryValue() const;
  Real computeOutflowBoundaryValueRHSContribution() const;
  Real computeBackflowBoundaryValue() const;
  Real computeBackflowBoundaryValueMatrixContribution() const;
  Real computeVelocity(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  RealVectorValue cellVelocity(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  RealVectorValue outwardUnitNormal() const;
  RealGradient computeVelocityGradient(const ElemInfo & elem_info,
                                       const Moose::StateArg & state) const;

  const unsigned int _dim;
  const MooseLinearVariableFVReal * const _u_var;
  const MooseLinearVariableFVReal * const _v_var;
  const MooseLinearVariableFVReal * const _w_var;
  std::vector<const MooseLinearVariableFVReal *> _velocity_vars;
  const unsigned int _index;
  const Moose::Functor<Real> & _backflow_value;
  const Moose::Functor<Real> & _face_flux;
  const bool _two_term_expansion;
};
