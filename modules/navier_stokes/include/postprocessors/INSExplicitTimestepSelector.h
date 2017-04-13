/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSEXPLICITTIMESTEPSELECTOR_H
#define INSEXPLICITTIMESTEPSELECTOR_H

#include "ElementPostprocessor.h"

class INSExplicitTimestepSelector;

template <>
InputParameters validParams<INSExplicitTimestepSelector>();

/**
 * Postprocessor that computes the minimum value of h_min/|u|,
 * where |u| is coupled in as an aux variable.
 */
class INSExplicitTimestepSelector : public ElementPostprocessor
{
public:
  INSExplicitTimestepSelector(const InputParameters & parameters);
  virtual ~INSExplicitTimestepSelector();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & uo);

protected:
  /// The value of dt (NOTE: _dt member variable is already defined)
  Real _value;

  /// Velocity magnitude.  Hint: Use VectorMagnitudeAux in Moose for this
  const VariableValue & _vel_mag;

  /// Material properties:  the explicit time scheme limit for the viscous
  /// problem also depends on the kinematic viscosity.
  Real _mu;
  Real _rho;

  /// We can compute maximum stable timesteps based on the linearized
  /// theory, but even those timesteps are sometimes still too large
  /// for explicit timestepping in a "real" problem.  Therefore, we
  /// provide an additional "fudge" factor, 0 < beta < 1, that can be
  /// used to reduce the selected timestep even further.
  Real _beta;
};

#endif /* INSEXPLICITTIMESTEPSELECTOR_H */
