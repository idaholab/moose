/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef OUTPUTRGB_H
#define OUTPUTRGB_H

#include "AuxKernel.h"
#include "EulerAngleProvider.h"
#include "GrainTracker.h"

//Forward Declarations
class OutputRGB;
class GrainTracker;
class EulerAngleProvider;

template<>
InputParameters validParams<OutputRGB>();

/**
 * Output euler angles from user object to an AuxVariable.
 */
class OutputRGB : public AuxKernel
{
public:
  OutputRGB(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Reference direction of the sample
  const unsigned int _sd;

  /// Crystal structure of the sample
  const unsigned int _xtal_class;

  /// Object providing the Euler angles
  const EulerAngleProvider & _euler;

  /// Grain tracker object
  const GrainTracker & _grain_tracker;
};

#endif //OUTPUTRGB_H
