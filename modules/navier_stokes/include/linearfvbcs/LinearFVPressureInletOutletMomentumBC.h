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
  RealVectorValue outwardUnitNormal() const;

  const unsigned int _dim;
  const NS::LinearFVVelocityVariableArray _velocity_vars;
  const unsigned int _index;
};
