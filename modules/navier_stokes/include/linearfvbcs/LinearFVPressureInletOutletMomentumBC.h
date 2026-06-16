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
#include "NS.h"

class ElemInfo;

/**
 * Pressure-controlled inlet/outlet velocity boundary condition for the sharp-interface path.
 *
 * The historical class name is retained, but the solved variable and returned boundary values are
 * velocity components, not rho*u.
 */
class LinearFVPressureInletOutletMomentumBC : public LinearFVInletOutletScalarBC
{
public:
  static InputParameters validParams();

  LinearFVPressureInletOutletMomentumBC(const InputParameters & parameters);

protected:
  Real computeBackflowBoundaryValue() const override;
  Real computeBackflowBoundaryValueMatrixContribution() const override;
  RealVectorValue cellVelocity(const ElemInfo & elem_info, const Moose::StateArg & state) const;
  RealVectorValue outwardUnitNormal() const;

  const unsigned int _dim;
  const MooseLinearVariableFVReal * const _u_var;
  const MooseLinearVariableFVReal * const _v_var;
  const MooseLinearVariableFVReal * const _w_var;
  std::vector<const MooseLinearVariableFVReal *> _velocity_vars;
  const unsigned int _index;
};
