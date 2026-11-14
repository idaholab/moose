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
#include "NSFVUtils.h"

#include "libmesh/point.h"

/**
 * Linear-FV total-pressure boundary condition for a reduced-pressure solve.
 *
 * This is a fixed-value p_rgh boundary. It applies the supplied static/total pressure reference,
 * subtracts the p_rgh hydrostatic offset, and on backflow subtracts the incoming dynamic pressure:
 *   p_rgh_b = p_ref - rho g.(x_f - x_ref) - [backflow] rho (u_P.n)^2 / 2.
 * The hydrostatic offset is what turns a prescribed physical pressure, such as the atmosphere at
 * an open top, into the reduced pressure the equation is solved for; without it the boundary
 * would impose a physical pressure that varies with height. The dynamic part keeps fluid that is
 * drawn in through the boundary from being accelerated by the full reference pressure, which
 * would feed an inflow that grows on itself.
 */
class LinearFVPrghTotalPressureBC : public LinearFVAdvectionDiffusionFunctorDirichletBC
{
public:
  static InputParameters validParams();

  LinearFVPrghTotalPressureBC(const InputParameters & parameters);

  Real computeBoundaryValue() const override;
  bool needsBoundaryNonorthogonalCorrection() const override { return false; }

protected:
  /// Whether the face flux enters the domain through this face
  bool isBackflow() const;
  /// rho (u_P.n)^2 / 2 of the entering fluid on backflow, zero on outflow
  Real dynamicPressureCorrection() const;
  /// rho g.(x_f - x_ref) with the buoyancy density, the difference between p and p_rgh
  Real hydrostaticPressureOffset() const;

  const unsigned int _dim;
  const NS::LinearFVVelocityVariableArray _velocity_vars;
  /// The density in the backflow dynamic pressure
  const Moose::Functor<Real> & _density;
  /// The density in the hydrostatic offset
  const Moose::Functor<Real> & _buoyancy_density;
  /// The face flux whose sign tells outflow from backflow
  const Moose::Functor<Real> & _face_flux;
  const RealVectorValue _gravity;
  const Point _reference_pressure_point;
};
