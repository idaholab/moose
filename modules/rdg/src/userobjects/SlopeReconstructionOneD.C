/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SlopeReconstructionOneD.h"

template <>
InputParameters
validParams<SlopeReconstructionOneD>()
{
  InputParameters params = validParams<SlopeReconstructionBase>();
  return params;
}

SlopeReconstructionOneD::SlopeReconstructionOneD(const InputParameters & parameters)
  : SlopeReconstructionBase(parameters)
{
}
