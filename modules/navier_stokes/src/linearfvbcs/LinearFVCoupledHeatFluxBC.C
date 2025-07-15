//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVCoupledHeatFluxBC.h"

registerMooseObject("MooseApp", LinearFVCoupledHeatFluxBC);

InputParameters
LinearFVCoupledHeatFluxBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a fixed diffusive flux BC which can be used for the assembly of linear "
      "finite volume system and whose normal face gradient values are determined "
      "using a functor. This kernel is only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>(
      "vhtc", "The virtual heat transfer coefficient for this BC.");
  params.addRequiredParam<MooseFunctorName>("solid_conductivity", "The solid conductivity for the variable.");
  params.addRequiredParam<MooseFunctorName>("fluid_conductivity", "The fluid conductivity for the variable.");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "The fluid temperature variable");
  params.addRequiredParam<MooseFunctorName>(NS::T_solid, "The solid/wall temperature variable");
  return params;
}

LinearFVCoupledHeatFluxBC::LinearFVCoupledHeatFluxBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _vhtc(getFunctor<Real>("vhtc")),
    _solid_conductivity(getFunctor<Real>("solid_conductivity")),
    _fluid_conductivity(getFunctor<Real>("fluid_conductivity")),
    _temp_fluid(getFunctor<Real>(NS::T_fluid)),
    _temp_solid(getFunctor<Real>(NS::T_solid)),
    _var_is_fluid("wraps_" + _var.name() == _temp_fluid.functorName() ||
       "wraps_" + _var.name() + "_raw_value" == _temp_fluid.functorName())
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
LinearFVCoupledHeatFluxBC::computeCoupledGradientValue() const
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

  return multiplier * (*_rhs_conductivity)(face,state) *
        (*_rhs_temperature).gradient(face,state)*_current_face_info->normal();
}

Real
LinearFVCoupledHeatFluxBC::computeBoundaryValue() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  // For non-orthogonal meshes we compute an extra correction vector to increase order accuracy
  // correction_vector is a vector orthogonal to the boundary normal
  const auto correction_vector =
      (d_cf - (d_cf * _current_face_info->normal()) * _current_face_info->normal());

  return raw_value(_var(elem_arg, determineState())) +
         computeCoupledGradientValue() / distance +
         _var.gradSln(*_current_face_info->elemInfo()) * correction_vector;
}

Real
LinearFVCoupledHeatFluxBC::computeBoundaryNormalGradient() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  return computeCoupledGradientValue();
}

Real
LinearFVCoupledHeatFluxBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVCoupledHeatFluxBC::computeBoundaryValueRHSContribution() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  // Fetch the boundary value from the provided functor.
  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  // For non-orthogonal meshes we compute an extra correction vector to increase order accuracy
  // correction_vector is a vector orthogonal to the boundary normal
  const auto correction_vector =
      (d_cf - (d_cf * _current_face_info->normal()) * _current_face_info->normal());
  return computeCoupledGradientValue() / distance +
         _var.gradSln(*_current_face_info->elemInfo()) * correction_vector;
}

Real
LinearFVCoupledHeatFluxBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVCoupledHeatFluxBC::computeBoundaryGradientRHSContribution() const
{
  return computeCoupledGradientValue();
}
