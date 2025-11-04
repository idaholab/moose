//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumMeshAdvection.h"

registerMooseObject("NavierStokesApp", INSFVMomentumMeshAdvection);

InputParameters
INSFVMomentumMeshAdvection::validParams()
{
  InputParameters params = INSFVMeshAdvection::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription(
      "Implements a momentum source/sink term proportional to the divergence of the mesh velocity");
  params.suppressParameter<MooseFunctorName>("advected_quantity");
  params.addParam<bool>(
      "add_to_a", true, "Whether to add this object's contribution to the Rhie-Chow coefficients");
  return params;
}

INSFVMomentumMeshAdvection::INSFVMomentumMeshAdvection(const InputParameters & parameters)
  : INSFVMeshAdvection(parameters),
    INSFVMomentumResidualObject(*this),
    _add_to_a(getParam<bool>("add_to_a"))
{
}

void
INSFVMomentumMeshAdvection::gatherRCData(const Elem & elem)
{
  if (_add_to_a)
  {
    const auto elem_arg = makeElemArg(&elem);
    const auto state = determineState();
    _rc_uo.addToA(&elem, _index, advQuantCoeff(elem_arg, state));
  }
}
