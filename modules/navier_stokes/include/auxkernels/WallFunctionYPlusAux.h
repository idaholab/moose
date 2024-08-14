//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include "INSFVVelocityVariable.h"
/**
 * Computes wall y+ based on wall functions.
 */
class WallFunctionYPlusAux : public AuxKernel
{
public:
  static InputParameters validParams();

  WallFunctionYPlusAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

    /// Equivalent sand height wall roughness
  const Real _rough_ks;

  /// Curvature radius
  const Moose::Functor<ADReal> * _curv_R;

  /// Wall convexity parameter
  const bool _convex;

  /// Curvature axis
  const Moose::Functor<ADReal> * _x_curvature_axis;
  const Moose::Functor<ADReal> * _y_curvature_axis;
  const Moose::Functor<ADReal> * _z_curvature_axis;
};
