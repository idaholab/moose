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
#include "EBSDReader.h"
#include "EulerAngleProvider.h"
#include "GrainTrackerInterface.h"

// Forward Declarations
class EulerAngleProvider;

/**
 * Output euler angles from user object to an AuxVariable.
 */
class EulerAngleProvider2RGBAux : public AuxKernel
{
public:
  static InputParameters validParams();

  EulerAngleProvider2RGBAux(const InputParameters & parameters);
  virtual unsigned int getNumGrains() const;

protected:
  virtual Real computeValue();
  virtual void precalculateValue();

  /// Optional phase number needed for global grain index retrieval
  const unsigned int _phase;

  /// Reference direction of the sample
  const unsigned int _sd;

  /// Crystal structure of the sample
  const unsigned int _xtal_class;

  /// Type of value to be outputted
  const unsigned int _output_type;

  /// Object providing the Euler angles
  const EulerAngleProvider & _euler;

  /// EBSDReader Object
  const EBSDReader * const _ebsd_reader;

  /// Grain tracker object
  const GrainTrackerInterface & _grain_tracker;

  /// precalculated element value
  Real _value;

  /// Vector containing values for color in regions without grains
  const Point _no_grain_color;
};
