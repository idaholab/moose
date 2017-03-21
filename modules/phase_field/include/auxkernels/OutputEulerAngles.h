/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef OUTPUTEULERANGLES_H
#define OUTPUTEULERANGLES_H

#include "AuxKernel.h"
#include "EulerAngleProvider.h"
#include "GrainTracker.h"

// Forward Declarations
class OutputEulerAngles;
class GrainTracker;
class EulerAngleProvider;

template <>
InputParameters validParams<OutputEulerAngles>();

/**
 * Output euler angles from user object to an AuxVariable.
 */
class OutputEulerAngles : public AuxKernel
{
public:
  OutputEulerAngles(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  virtual void precalculateValue();

  /// Object providing the Euler angles
  const EulerAngleProvider & _euler;

  /// Grain tracker object
  const GrainTracker & _grain_tracker;

  /// Number of grains
  MooseEnum _output_euler_angle;

  /// precalculated element value
  Real _value;
};

#endif // OUTPUTEULERANGLES_H
