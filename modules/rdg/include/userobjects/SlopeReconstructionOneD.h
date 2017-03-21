/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SLOPERECONSTRUCTIONONED_H
#define SLOPERECONSTRUCTIONONED_H

#include "SlopeReconstructionBase.h"

// Forward Declarations
class SlopeReconstructionOneD;

template <>
InputParameters validParams<SlopeReconstructionOneD>();

/**
 * One-dimensional piecewise linear slope reconstruction
 * to get the slopes of cell average variables
 */
class SlopeReconstructionOneD : public SlopeReconstructionBase
{
public:
  SlopeReconstructionOneD(const InputParameters & parameters);
};

#endif
