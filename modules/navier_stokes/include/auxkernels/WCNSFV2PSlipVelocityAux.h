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

class INSFVVelocityVariable;

/*
 * Computes the value of slip velocity for the two phase mixture model
 */
class WCNSFV2PSlipVelocityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  WCNSFV2PSlipVelocityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Continuous Phase Density
  const Moose::Functor<ADReal> & _rho_mixture;

  /// Dispersed Phase Density
  const Moose::Functor<ADReal> & _rho_d;

  /// Mixture Density
  const Moose::Functor<ADReal> & _mu_mixture;

  // Gravity acceleration vector
  RealVectorValue _gravity;

  /// Force Scale factor
  const Real & _force_scale;

  /// Force optional function value
  const Function & _force_function;

  /// Optional Force Postprocessor value
  const PostprocessorValue & _force_postprocessor;

  // Gravity acceleration vector
  RealVectorValue _force_direction;

  /// The linear friction factor, for laminar flow
  const Moose::Functor<ADReal> & _linear_friction;

  /// Particle diameter in the dispersed phase
  const Moose::Functor<ADReal> & _particle_diameter;

  /// index of the velocity component x|y|z
  unsigned int _index;
};
