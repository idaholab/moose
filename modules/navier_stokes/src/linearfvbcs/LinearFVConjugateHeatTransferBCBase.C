//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConjugateHeatTransferBCBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVConjugateHeatTransferBC);

InputParameters
LinearFVConjugateHeatTransferBCBase::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addRequiredParam<MooseFunctorName>("thermal_conductivity",
                                            "The solid conductivity for the variable.");
  params.addClassDescription("Class describing a conjugate heat transfer between two domains.");
  return params;
}

LinearFVConjugateHeatTransferBCBase::LinearFVConjugateHeatTransferBCBase(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _thermal_conductivity(getFunctor<Real>("thermal_conductivity"))
{
  _var.computeCellGradients();
}

Real
LinearFVConjugateHeatTransferBCBase::computeBoundaryConductionFlux() const
{
  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto boundary_normal_multiplier =
      (_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR) ? 1.0 : -1.0;

  return boundary_normal_multiplier * _var.gradSln(*elem_info) * _current_face_info->normal();
}
