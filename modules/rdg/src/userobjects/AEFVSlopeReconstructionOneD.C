//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
