//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDirichletConjugateHeatTransferBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVDirichletConjugateHeatTransferBC);

InputParameters
LinearFVDirichletConjugateHeatTransferBC::validParams()
{
  InputParameters params = LinearFVConjugateHeatTransferBCBase::validParams();
  params.addRequiredParam<MooseFunctorName>("incoming_temperature", "Blabla");
  params.addClassDescription("Class describing a conjugate heat transfer between two domains.");
  return params;
}

LinearFVDirichletConjugateHeatTransferBC::LinearFVDirichletConjugateHeatTransferBC(
    const InputParameters & parameters)
  : LinearFVConjugateHeatTransferBCBase(parameters),
    _incoming_temperature(getFunctor<Real>("incoming_temperature"))
{
  _var.computeCellGradients();
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryConductionFlux() const
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

  return _thermal_conductivity(singleSidedFaceArg(_current_face_info), determineState()) *
             (_incoming_temperature(singleSidedFaceArg(_current_face_info), determineState()) -
              _var.getElemValue(*elem_info, determineState())) /
             computeCellToFaceDistance() +
         _var.gradSln(*elem_info) * correction_vector;
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryValue() const
{
  // std::cout << "Computing boundary value on CHTDirichlet for " << _var.name() << " "
  //           << _incoming_temperature(singleSidedFaceArg(_current_face_info), determineState())
  //           << std::endl;
  return _incoming_temperature(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryNormalGradient() const
{
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();
  return (_incoming_temperature(singleSidedFaceArg(_current_face_info), determineState()) -
          raw_value(_var(elem_arg, determineState()))) /
         distance;
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryValueMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryValueRHSContribution() const
{
  return _incoming_temperature(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryGradientMatrixContribution() const
{
  return 1.0 / computeCellToFaceDistance();
}

Real
LinearFVDirichletConjugateHeatTransferBC::computeBoundaryGradientRHSContribution() const
{
  // std::cout << "Solid temperature in fluid "
  //           << _incoming_temperature(singleSidedFaceArg(_current_face_info), determineState())
  //           << std::endl;
  return _incoming_temperature(singleSidedFaceArg(_current_face_info), determineState()) /
         computeCellToFaceDistance();
}
