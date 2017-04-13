/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVSlopeReconstructionOneD.h"

template <>
InputParameters
validParams<CNSFVSlopeReconstructionOneD>()
{
  InputParameters params = validParams<SlopeReconstructionOneD>();
  params.addClassDescription("A user object that performs piecewise linear slope reconstruction to "
                             "get the slopes of cell average variables in 1-D, though nothing "
                             "needs to be done in this class because slope limiter can be done all "
                             "in the 1-D slope limiting class");
  return params;
}

CNSFVSlopeReconstructionOneD::CNSFVSlopeReconstructionOneD(const InputParameters & parameters)
  : SlopeReconstructionOneD(parameters)
{
}

void
CNSFVSlopeReconstructionOneD::reconstructElementSlope()
{
}
