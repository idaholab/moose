//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDirichletCHTBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVDirichletCHTBC);

InputParameters
LinearFVDirichletCHTBC::validParams()
{
  InputParameters params = LinearFVCHTBCBase::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "thermal_conductivity",
      "Thermal conductivity, mainly used for the diffusive flux computation.");
  params.addRequiredParam<MooseFunctorName>(
      "prescribed_temperature",
      "Functor describing the temperature which is being prescibed on the boundary");
  params.addClassDescription(
      "Conjugate heat transfer BC for Dirichlet boundary condition-based coupling.");
  // params.suppressParameter<bool>("flux_relaxation");
  return params;
}

LinearFVDirichletCHTBC::LinearFVDirichletCHTBC(const InputParameters & parameters)
  : LinearFVCHTBCBase(parameters),
    _thermal_conductivity(getFunctor<Real>("thermal_conductivity")),
    _prescribed_temperature(getFunctor<Real>("prescribed_temperature"))
{
}

Real
LinearFVDirichletCHTBC::computeBoundaryConductionFlux() const
{
  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto boundary_normal_multiplier =
      (_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR) ? 1.0 : -1.0;

  const auto correction_vector =
      boundary_normal_multiplier * _current_face_info->normal() -
      1 /
          (boundary_normal_multiplier * _current_face_info->normal() *
           (_current_face_info->faceCentroid() - elem_info->centroid())) *
          (_current_face_info->faceCentroid() - elem_info->centroid());

  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  return -_thermal_conductivity(face, state) *
         ((_prescribed_temperature(face, state) - _var.getElemValue(*elem_info, state)) /
              computeCellToFaceDistance() +
          _var.gradSln(*elem_info) * correction_vector);
}

Real
LinearFVDirichletCHTBC::computeBoundaryValue() const
{
  return _prescribed_temperature(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVDirichletCHTBC::computeBoundaryNormalGradient() const
{
  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const Real distance = computeCellToFaceDistance();
  const auto state = determineState();
  return (_prescribed_temperature(singleSidedFaceArg(_current_face_info), state) -
          _var.getElemValue(*elem_info, state)) /
         distance;
}

Real
LinearFVDirichletCHTBC::computeBoundaryValueMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVDirichletCHTBC::computeBoundaryValueRHSContribution() const
{
  return _prescribed_temperature(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVDirichletCHTBC::computeBoundaryGradientMatrixContribution() const
{
  return 1.0 / computeCellToFaceDistance();
}

Real
LinearFVDirichletCHTBC::computeBoundaryGradientRHSContribution() const
{
  return _prescribed_temperature(singleSidedFaceArg(_current_face_info), determineState()) /
         computeCellToFaceDistance();
}
