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
LinearFVConjugateHeatTransferBC::computeBoundaryValueMatrixContribution() const
{
  // We approximate the face value with the cell value here.
  // TODO: we can extend this to a 2-term expansion at some point when the need arises.
  return 1.0;
}

Real
LinearFVConjugateHeatTransferBC::computeBoundaryValueRHSContribution() const
{
  // We approximate the face value with the cell value, we
  // don't need to add anything to the right hand side.
  return 0.0;
}

Real
LinearFVConjugateHeatTransferBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  // We just put the heat transfer coefficient on the diagonal (multiplication with the
  // surface area is taken care of in the kernel).
  return  _htc(face, state);
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

  const auto neighbor_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                 ? _current_face_info->neighborInfo()
                                 : _current_face_info->elemInfo();

  const auto fluid_side_elem_info = _var_is_fluid ? elem_info : neighbor_info;

  const auto multiplier = _current_face_info->normal() *
	  (_current_face_info->faceCentroid() - fluid_side_elem_info->centroid()) > 0
                          ?  1
                          : -1;

  const auto q_prev = multiplier * (*_rhs_conductivity)(face,state) *
        (*_rhs_temperature).gradient(face,state)*_current_face_info->normal();

  const auto t_prev = (*_rhs_temperature)(face,state);


  return _htc(face, state) * t_prev - q_prev;
}
