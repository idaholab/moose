//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConjugateHeatTransferBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVConjugateHeatTransferBC);

InputParameters
LinearFVConjugateHeatTransferBC::validParams()
{
  InputParameters params = LinearFVConvectiveHeatTransferBC::validParams();
  params.addRequiredParam<MooseFunctorName>("solid_conductivity", "The solid conductivity for the variable.");
  params.addRequiredParam<MooseFunctorName>("fluid_conductivity", "The fluid conductivity for the variable.");
  params.addClassDescription("Class describing a conjugate heat transfer between two domains.");
  return params;
}

LinearFVConjugateHeatTransferBC::LinearFVConjugateHeatTransferBC(
  const InputParameters & parameters)
  : LinearFVConvectiveHeatTransferBC(parameters),
    _solid_conductivity(getFunctor<Real>("solid_conductivity")),
    _fluid_conductivity(getFunctor<Real>("fluid_conductivity"))
{
  // We determine the alpha functor's sign for the Robin BC coupling
  if (_var_is_fluid)
  {
    _rhs_temperature  = &_temp_solid;
    _rhs_conductivity = &_solid_conductivity;
  }
  else
  {
    _rhs_temperature  = &_temp_fluid;
    _rhs_conductivity = &_fluid_conductivity;
  }

  _var.computeCellGradients();
}

Real
LinearFVConjugateHeatTransferBC::computeBoundaryGradientRHSContribution() const
{
  // We check where the functor contributing to the right hand side lives. We do this
  // because this functor lives on the domain where the variable of this kernel doesn't.
  const auto state = determineState();
  auto face = singleSidedFaceArg(_current_face_info);

  if (_rhs_temperature->hasFaceSide(*_current_face_info, true))
    face.face_side = _current_face_info->elemPtr();
  else
    face.face_side = _current_face_info->neighborPtr();

  // TODO: check if includesMaterialPropertyMultiplier() affects this... it shouldn't

  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  // const auto multiplier = _current_face_info->normal() * (_current_face_info->faceCentroid() -
  //                                                         fluid_side_elem_info->centroid()) >
  //                                 0
  //                             ? 1
  //                             : -1;
  const auto t_coupled = (*_rhs_temperature)(face, state) -
              (  (*_rhs_conductivity)(face,state) *
                  (*_rhs_temperature).gradient(face,state)*_current_face_info->normal()
              / _htc(face, state));

  return  _htc(face, state) * t_coupled;
}
