//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVNaturalFreeSlipBC.h"

#include "INSFVVelocityVariable.h"
/**
 * A class for setting the wall shear stress at the walls, based on
 * the standard wall function formulation.
 */
class INSFVWallFunctionBC : public INSFVNaturalFreeSlipBC
{
public:
  static InputParameters validParams();
  INSFVWallFunctionBC(const InputParameters & params);

  using INSFVNaturalFreeSlipBC::gatherRCData;
  void gatherRCData(const FaceInfo &) override final;

protected:
  ADReal computeStrongResidual();

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// density
  const Moose::Functor<ADReal> & _rho;

  /// dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Rhie-Chow coefficient
  ADReal _a = 0;
};
