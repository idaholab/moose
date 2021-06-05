//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVPressureVariable.h"

registerMooseObject("NavierStokesApp", INSFVPressureVariable);

InputParameters
INSFVPressureVariable::validParams()
{
  InputParameters params = INSFVVariable::validParams();

  // The pressure gradient is used by INSFVMomentumPressure and by Rhie Chow interpolation
  // in INSFVMomentumAdvection also for each direction.
  params.set<bool>("cache_face_gradients") = true;

  return params;
}

INSFVPressureVariable::INSFVPressureVariable(const InputParameters & params) : INSFVVariable(params)
{
}
