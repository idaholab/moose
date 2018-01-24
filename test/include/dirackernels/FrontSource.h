//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FRONTSOURCE_H
#define FRONTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

#include "TrackDiracFront.h"

// Forward Declarations
class FrontSource;

template <>
InputParameters validParams<FrontSource>();

/**
 * An example showing how point sources can be applied based on
 * where a moving front is.  To do this we use a NodalUserObject to pick
 * the points where the sources should be applied.
 */
class FrontSource : public DiracKernel
{
public:
  FrontSource(const InputParameters & parameters);
  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const Real & _value;

  const TrackDiracFront & _front_tracker;
};

#endif // FRONTSOURCE_H
