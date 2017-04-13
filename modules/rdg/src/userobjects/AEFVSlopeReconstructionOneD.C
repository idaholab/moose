/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AEFVSlopeReconstructionOneD.h"

template <>
InputParameters
validParams<AEFVSlopeReconstructionOneD>()
{
  InputParameters params = validParams<SlopeReconstructionOneD>();
  params.addClassDescription("One-dimensional piecewise linear slope reconstruction to get the "
                             "slope of cell average variable for the advection equation using a "
                             "cell-centered finite volume method.");
  return params;
}

AEFVSlopeReconstructionOneD::AEFVSlopeReconstructionOneD(const InputParameters & parameters)
  : SlopeReconstructionOneD(parameters)
{
}

void
AEFVSlopeReconstructionOneD::reconstructElementSlope()
{
}
