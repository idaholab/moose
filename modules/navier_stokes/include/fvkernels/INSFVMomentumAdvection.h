//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvection.h"
#include "TheWarehouse.h"
#include "SubProblem.h"
#include "MooseApp.h"
#include "INSFVAttributes.h"
#include "NSFVRhieChowInterpolator.h"

#include <vector>
#include <set>

class INSFVVelocityVariable;
class INSFVPressureVariable;

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class INSFVMomentumAdvection : public FVMatAdvection, UserObjectInterface
{
public:
  static InputParameters validParams();
  INSFVMomentumAdvection(const InputParameters & params);
  void initialSetup() override;

protected:

  virtual ADReal computeQpResidual() override;

  void residualSetup() override final { //clearRCCoeffs();
   }
  void jacobianSetup() override final { //clearRCCoeffs();
   }

  bool skipForBoundary(const FaceInfo & fi) const override;

  /// pressure variable
  const INSFVPressureVariable * const _p_var;
  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Density
  const Real & _rho;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;

private:

  /// User object for computing velocity on element faces
  const NSFVRhieChowInterpolator * _velocity_interpolator;
};
