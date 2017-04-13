/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVSLOPERECONSTRUCTIONONED_H
#define CNSFVSLOPERECONSTRUCTIONONED_H

#include "SlopeReconstructionOneD.h"

// Forward Declarations
class CNSFVSlopeReconstructionOneD;

template <>
InputParameters validParams<CNSFVSlopeReconstructionOneD>();

/**
 * A user object that performs piecewise linear slope reconstruction to get the slopes of cell
 * average variables in 1-D
 *
 * Notes: nothing needs to be done in this class because slope limiter can be done all in the 1-D
 * slope limiting class
 */
class CNSFVSlopeReconstructionOneD : public SlopeReconstructionOneD
{
public:
  CNSFVSlopeReconstructionOneD(const InputParameters & parameters);

  /// compute the slope of the cell
  virtual void reconstructElementSlope();

protected:
};

#endif
