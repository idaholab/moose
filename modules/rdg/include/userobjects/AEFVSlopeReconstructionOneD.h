//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AEFVSLOPERECONSTRUCTIONONED_H
#define AEFVSLOPERECONSTRUCTIONONED_H

#include "SlopeReconstructionOneD.h"

// Forward Declarations
class AEFVSlopeReconstructionOneD;

template <>
InputParameters validParams<AEFVSlopeReconstructionOneD>();

/**
 * One-dimensional piecewise linear slope reconstruction
 * to get the slope of cell average variable
 * for the advection equation
 * using a cell-centered finite volume method
 *
 * Notes: nothing needs to be done in this class
 *        because one-d slope limiter can be done
 *        all in the one-d slope limiting class
 */
class AEFVSlopeReconstructionOneD : public SlopeReconstructionOneD
{
public:
  AEFVSlopeReconstructionOneD(const InputParameters & parameters);

  /// compute the slope of the cell
  virtual void reconstructElementSlope() override;

protected:
};

#endif
