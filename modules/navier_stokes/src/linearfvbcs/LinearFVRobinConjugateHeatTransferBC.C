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
  params.addRequiredParam<MooseFunctorName>("h", "The convective heat transfer coefficient");
  params.addRequiredParam<MooseFunctorName>("incoming_flux", "Blabla");
  params.addRequiredParam<MooseFunctorName>("incoming_temperature", "Blabla");

  params.addClassDescription("Class describing a conjugate heat transfer between two domains.");
  return params;
}

LinearFVRobinConjugateHeatTransferBC::LinearFVRobinConjugateHeatTransferBC(
    const InputParameters & parameters)
  : LinearFVConjugateHeatTransferBCBase(parameters),
    _htc(getFunctor<Real>("h")),
    _incoming_flux(&getFunctor<Real>("incoming_flux")),
    _incoming_temperature(&getFunctor<Real>("incoming_temperature"))
{
  _var.computeCellGradients();
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryConductionFlux() const
{
  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

  const auto state = determineState();
  auto face = singleSidedFaceArg(_current_face_info);
  face.face_side = elem_info->elem();

  // std::cout << "Computing flux for coupling on  " << _var.name() << " " << _htc(face, state) << "
  // "
  //           << _var.getElemValue(*elem_info, state) << " " << (*_incoming_temperature)(face,
  //           state)
  //           << " " << -(*_incoming_flux)(face, state) << " "
  //           << _htc(face, state) * (_var.getElemValue(*elem_info, state) -
  //                                   (*_incoming_temperature)(face, state)) -
  //                  (*_incoming_flux)(face, state)
  //           << std::endl;

  return _htc(face, state) *
             (_var.getElemValue(*elem_info, state) - (*_incoming_temperature)(face, state)) -
         (*_incoming_flux)(face, state);
}

Real
LinearFVRobinConjugateHeatTransferBC::computeBoundaryValue() const
{

  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  // std::cout << "Computing boundary value " << _var.name() << " "
  //           << _var.getElemValue(*elem_info, determineState()) << std::endl;

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

  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

  const auto state = determineState();
  auto face = singleSidedFaceArg(_current_face_info);
  face.face_side = elem_info->elem();

  // std::cout << "Applying face flux " << _var.name() << " "
  //           << _htc(face, state) * (*_incoming_temperature)(face, state) << " "
  //           << (*_incoming_flux)(face, state) << " "
  //           << _htc(face, state) * (*_incoming_temperature)(face, state) +
  //                  (*_incoming_flux)(face, state)
  //           << std::endl;

  return _htc(face, state) * (*_incoming_temperature)(face, state) + (*_incoming_flux)(face, state);
}
