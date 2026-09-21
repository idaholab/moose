//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class INSFVVelocityVariable;

/**
 * Computes the value of slip velocity for the two phase mixture model
 */
class WCNSFV2PSlipVelocityFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  WCNSFV2PSlipVelocityFunctorMaterial(const InputParameters & parameters);

protected:
  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  MooseVariableField<Real> * const _u_var;
  /// y-velocity
  MooseVariableField<Real> * const _v_var;
  /// z-velocity
  MooseVariableField<Real> * const _w_var;

  /// Mixture density
  const Moose::Functor<ADReal> & _rho_mixture;

  /// Dispersed Phase Density
  const Moose::Functor<ADReal> & _rho_d;

  /// Continuous phase dynamic viscosity (the member keeps its historical name)
  const Moose::Functor<ADReal> & _mu_mixture;

  // Gravity acceleration vector
  RealVectorValue _gravity;

  /// Force scale factor
  const Real & _force_scale;

  /// Force optional function value
  const Function & _force_function;

  /// Optional Force Postprocessor value
  const PostprocessorValue & _force_postprocessor;

  /// Force direction vector
  RealVectorValue _force_direction;

  /// The linear friction factor, for laminar flow
  const Moose::Functor<ADReal> & _linear_friction;

  /// Whether the drag is closed by the correlation solved together with the force balance
  const bool _use_drag_model;

  /// Continuous phase density, for the drag model's Reynolds number
  const Moose::Functor<ADReal> * const _rho_c;

  /// Particle diameter in the dispersed phase
  const Moose::Functor<ADReal> & _particle_diameter;

  /// Dispersed phase fraction, converting the slip velocity into the drift velocity
  const Moose::Functor<ADReal> & _f_d;

  /// index of the velocity component x|y|z
  unsigned int _index;
};
