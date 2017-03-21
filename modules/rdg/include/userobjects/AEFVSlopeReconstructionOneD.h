/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
