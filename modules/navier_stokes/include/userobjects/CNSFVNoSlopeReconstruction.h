/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVNOSLOPERECONSTRUCTION_H
#define CNSFVNOSLOPERECONSTRUCTION_H

#include "SlopeReconstructionBase.h"

// Forward Declarations
class CNSFVNoSlopeReconstruction;

template <>
InputParameters validParams<CNSFVNoSlopeReconstruction>();

/**
 * A user object that does no slope reconstruction in multi-dimensions
 */
class CNSFVNoSlopeReconstruction : public SlopeReconstructionBase
{
public:
  CNSFVNoSlopeReconstruction(const InputParameters & parameters);

  /// compute the slope of the cell
  virtual void reconstructElementSlope();
};

#endif
