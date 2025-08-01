//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVRobinConjugateHeatTransferBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVRobinConjugateHeatTransferBC);

InputParameters
LinearFVRobinConjugateHeatTransferBC::validParams()
{
  InputParameters params = LinearFVConjugateHeatTransferBCBase::validParams();
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "The fluid temperature variable");
  params.addRequiredParam<MooseFunctorName>(NS::T_solid, "The solid/wall temperature variable");
  params.addRequiredParam<MooseFunctorName>("h", "The convective heat transfer coefficient");
  params.addRequiredParam<MooseFunctorName>("incoming_flux", "Blabla");
  params.addClassDescription("Class describing a conjugate heat transfer between two domains.");
  return params;
}

LinearFVRobinConjugateHeatTransferBC::LinearFVRobinConjugateHeatTransferBC(
    const InputParameters & parameters)
  : LinearFVConjugateHeatTransferBCBase(parameters),
    _incoming_flux(getFunctor<Real>("incoming_flux")),
    _htc(getFunctor<Real>("h"))
{
  _var.computeCellGradients();
}

void
LinearFVRobinConjugateHeatTransferBC::initialSetup()
{
  _temp_fluid = &getFunctor<Real>(NS::T_fluid);
  _temp_solid = &getFunctor<Real>(NS::T_solid);
  _var_is_fluid = "wraps_" + _var.name() == _temp_fluid->functorName() ||
                  "wraps_" + _var.name() + "_raw_value" == _temp_fluid->functorName();
  // We determine which one is the source variable
  if (_var_is_fluid)
    _rhs_temperature = _temp_solid;
  else
    _rhs_temperature = _temp_fluid;
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryValue() const
{
  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  return _var.getElemValue(*elem_info, determineState());
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryNormalGradient() const
{
  return 0.0;
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryValueMatrixContribution() const
{
  // We approximate the face value with the cell value here.
  // TODO: we can extend this to a 2-term expansion at some point when the need arises.
  return 1.0;
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryValueRHSContribution() const
{
  // We approximate the face value with the cell value, we
  // don't need to add anything to the right hand side.
  return 0.0;
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  // We just put the heat transfer coefficient on the diagonal (multiplication with the
  // surface area is taken care of in the kernel).
  return _htc(face, state);
}
Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryGradientRHSContribution() const
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
  return _htc(face, state) * (*_rhs_temperature)(face, state) - _incoming_flux(face, state);
}
