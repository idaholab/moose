//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"

#include "libmesh/point.h"

class ElemInfo;

/**
 * Linear-FV total-pressure boundary condition for a reduced-pressure solve.
 *
 * This is a fixed-value p_rgh boundary. It applies the supplied static/total pressure reference,
 * subtracts the p_rgh hydrostatic offset, and on backflow subtracts the incoming dynamic pressure.
 */
class LinearFVPrghTotalPressureBC : public LinearFVAdvectionDiffusionFunctorDirichletBC
{
public:
  static InputParameters validParams();

  LinearFVPrghTotalPressureBC(const InputParameters & parameters);

  Real computeBoundaryValue() const override;
  bool useBoundaryGradientExtrapolation() const override { return false; }

protected:
  const ElemInfo & fluidElemInfo() const;
  bool isBackflow() const;
  Real outwardFaceFlux() const;
  RealVectorValue cellVelocity(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  Real dynamicPressureCorrection() const;
  Real hydrostaticPressureOffset() const;

  const unsigned int _dim;
  const MooseLinearVariableFVReal * const _u_var;
  const MooseLinearVariableFVReal * const _v_var;
  const MooseLinearVariableFVReal * const _w_var;
  std::vector<const MooseLinearVariableFVReal *> _velocity_vars;
  const Moose::Functor<Real> & _density;
  const Moose::Functor<Real> & _face_flux;
  const RealVectorValue _gravity;
  const Point _reference_pressure_point;
  const bool _use_normal_velocity_only;
};
