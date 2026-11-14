//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVInletOutletScalarBC.h"
#include "NSFVUtils.h"

/**
 * Velocity boundary condition of a pressure-controlled inlet/outlet: a zero-gradient outlet on
 * outflow, a fixed tangential velocity with an extrapolated normal velocity on backflow.
 *
 * On backflow the component i of the boundary velocity is
 *   u_b,i = n_i (u_P.n) + (1 - n_i^2) u_t,i,
 * the normal part taken from the cell so that the pressure, not this condition, sets how much
 * fluid enters, and the tangential part prescribed (zero by default) so that entering fluid does
 * not carry an arbitrary tangential velocity. The n_i^2 share of the value depends on the cell
 * and is kept in the matrix, see computeBackflowBoundaryValueMatrixContribution.
 */
class LinearFVPressureInletOutletVelocityBC : public LinearFVInletOutletScalarBC
{
public:
  static InputParameters validParams();

  LinearFVPressureInletOutletVelocityBC(const InputParameters & parameters);

protected:
  Real computeBackflowBoundaryValue() const override;
  Real computeBackflowBoundaryValueMatrixContribution() const override;

  const unsigned int _dim;
  const NS::LinearFVVelocityVariableArray _velocity_vars;
  /// The velocity component this condition applies to
  const unsigned int _index;
};
