//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include "TheWarehouse.h"
#include "SubProblem.h"
#include "MooseApp.h"
#include "INSFVAttributes.h"
#include "INSFVMomentumResidualObject.h"
#include "INSFVBCInterface.h"

#include <vector>
#include <set>

class INSFVVelocityVariable;
class INSFVPressureVariable;

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class INSFVMomentumAdvection : public FVFluxKernel,
                               public INSFVBCInterface,
                               public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVMomentumAdvection(const InputParameters & params);
  void initialSetup() override;
  void gatherRCData(const Elem &) override final {}
  void gatherRCData(const FaceInfo & fi) override final;

protected:
  virtual ADReal computeQpResidual() override;

  bool skipForBoundary(const FaceInfo & fi) const override;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;

  /// The a coefficient for the element
  ADReal _ae = 0;

  /// The a coefficient for the neighbor
  ADReal _an = 0;
};
