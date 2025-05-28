//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConvectiveHeatTransferBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVConvectiveHeatTransferBC);

InputParameters
LinearFVConvectiveHeatTransferBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "The fluid temperature variable");
  params.addRequiredParam<MooseFunctorName>(NS::T_solid, "The solid/wall temperature variable");
  params.addRequiredParam<MooseFunctorName>("h", "The convective heat transfer coefficient");
  params.addClassDescription("Class describing a convective heat transfer between two domains.");
  return params;
}

LinearFVConvectiveHeatTransferBC::LinearFVConvectiveHeatTransferBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _temp_fluid(getFunctor<Real>(NS::T_fluid)),
    _temp_solid(getFunctor<Real>(NS::T_solid)),
    _htc(getFunctor<Real>("h")),
    _var_is_fluid("wraps_" + _var.name() == _temp_fluid.functorName() ||
                  "wraps_" + _var.name() + "_raw_value" == _temp_fluid.functorName())
{
  // We determine which one is the source variable
  if (_var_is_fluid)
    _rhs_temperature = &_temp_solid;
  else
    _rhs_temperature = &_temp_fluid;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryValue() const
{
  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  return _var.getElemValue(*elem_info, determineState());
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryNormalGradient() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  const auto neighbor_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                 ? _current_face_info->neighborInfo()
                                 : _current_face_info->elemInfo();

  const auto fluid_side_elem_info = _var_is_fluid ? elem_info : neighbor_info;

  // All this fuss is just for cases when we have an internal boundary, then the flux will change
  // signs depending on which side of the face we are at.
  const auto multiplier = _current_face_info->normal() * (_current_face_info->faceCentroid() -
                                                          fluid_side_elem_info->centroid()) >
                                  0
                              ? 1
                              : -1;

  return multiplier * _htc(face, state) * (_temp_fluid(face, state) - _temp_solid(face, state));
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryValueMatrixContribution() const
{
  // We approximate the face value with the cell value here.
  // TODO: we can extend this to a 2-term expansion at some point when the need arises.
  return 1.0;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryValueRHSContribution() const
{
  // We approximate the face value with the cell value, we
  // don't need to add anything to the right hand side.
  // TODO: we can extend this to a 2-term expansion at some point when the need arises.
  return 0.0;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  // We just put the heat transfer coefficient on the diagonal (multiplication with the
  // surface area is taken care of in the kernel).
  return _htc(face, state);
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryGradientRHSContribution() const
{
  const auto state = determineState();
  auto face = singleSidedFaceArg(_current_face_info);

  // We check where the functor contributing to the right hand side lives. We do this
  // because this functor lives on the domain where the variable of this kernel doesn't.
  if (_rhs_temperature->hasFaceSide(*_current_face_info, true))
    face.face_side = _current_face_info->elemPtr();
  else
    face.face_side = _current_face_info->neighborPtr();

  return _htc(face, state) * (*_rhs_temperature)(face, state);
}
