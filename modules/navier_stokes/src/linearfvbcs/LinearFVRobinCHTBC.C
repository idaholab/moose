//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVRobinCHTBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVRobinCHTBC);

InputParameters
LinearFVRobinCHTBC::validParams()
{
  InputParameters params = LinearFVCHTBCInterface::validParams();
  params += LinearFVAdvectionDiffusionBC::validParams();
  params.addRequiredParam<MooseFunctorName>("h", "The convective heat transfer coefficient.");
  params.addRequiredParam<MooseFunctorName>("incoming_flux",
                                            "The incoming diffusive flux on the intervace.");
  params.addRequiredParam<MooseFunctorName>("surface_temperature",
                                            "The prescribed temperature on the interface.");

  params.addClassDescription(
      "Conjugate heat transfer BC for Robin boundary condition-based coupling.");
  return params;
}

LinearFVRobinCHTBC::LinearFVRobinCHTBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    LinearFVCHTBCInterface(),
    _htc(getFunctor<Real>("h")),
    _incoming_flux(getFunctor<Real>("incoming_flux")),
    _surface_temperature(getFunctor<Real>("surface_temperature"))
{
}

Real
LinearFVRobinCHTBC::computeBoundaryValue() const
{
  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  return _var.getElemValue(*elem_info, determineState());
}

Real
LinearFVRobinCHTBC::computeBoundaryNormalGradient() const
{
  // FIXME
  return 0.0;
}

Real
LinearFVRobinCHTBC::computeBoundaryValueMatrixContribution() const
{
  // We approximate the face value with the cell value here.
  // TODO: we can extend this to a 2-term expansion at some point when the need arises.
  return 1.0;
}

Real
LinearFVRobinCHTBC::computeBoundaryValueRHSContribution() const
{
  // We approximate the face value with the cell value, we
  // don't need to add anything to the right hand side.
  return 0.0;
}

Real
LinearFVRobinCHTBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  // We just put the heat transfer coefficient on the diagonal (multiplication with the
  // surface area is taken care of in the kernel).
  return _htc(face, state);
}
Real
LinearFVRobinCHTBC::computeBoundaryGradientRHSContribution() const
{
  // We check where the functor contributing to the right hand side lives. We do this
  // because this functor lives on the domain where the variable of this kernel doesn't.
  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

  const auto state = determineState();
  auto face = singleSidedFaceArg(_current_face_info);
  face.face_side = elem_info->elem();

  return _htc(face, state) * _surface_temperature(face, state) + _incoming_flux(face, state);
}
