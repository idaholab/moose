/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEPROVIDER2RGBAUX_H
#define EULERANGLEPROVIDER2RGBAUX_H

#include "AuxKernel.h"
#include "EulerAngleProvider.h"
#include "GrainTracker.h"

// Forward Declarations
class EulerAngleProvider2RGBAux;
class GrainTracker;
class EulerAngleProvider;

template <>
InputParameters validParams<EulerAngleProvider2RGBAux>();

/**
 * Output euler angles from user object to an AuxVariable.
 */
class EulerAngleProvider2RGBAux : public AuxKernel
{
public:
  EulerAngleProvider2RGBAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  virtual void precalculateValue();

  /// Reference direction of the sample
  const unsigned int _sd;

  /// Crystal structure of the sample
  const unsigned int _xtal_class;

  /// Type of value to be outputted
  const unsigned int _output_type;

  /// Object providing the Euler angles
  const EulerAngleProvider & _euler;

  /// Grain tracker object
  const GrainTracker & _grain_tracker;

  /// precalculated element value
  Real _value;

  /// Vector containing values for color in regions without grains
  const Point _no_grain_color;
};

#endif // EULERANGLEPROVIDER2RGBAUX_H
