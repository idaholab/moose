//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVInletOutletScalarBC.h"

registerMooseObject("NavierStokesApp", LinearFVInletOutletScalarBC);

InputParameters
LinearFVInletOutletScalarBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionOutflowBC::validParams();
  params.addClassDescription(
      "Adds an inlet-outlet-style BC for linear FV scalar transport. On outflow this behaves "
      "like a zero-gradient / extrapolated outlet; on backflow it switches to a prescribed "
      "boundary value.");
  params.addParam<MooseFunctorName>(
      "backflow_value",
      "0",
      "The boundary value imposed when the local boundary flow reverses and becomes inflow.");
  params.addParam<MooseFunctorName>(
      "face_flux",
      "corrected_face_phi",
      "The corrected face-flux functor used to switch between outflow and backflow.");
  return params;
}

LinearFVInletOutletScalarBC::LinearFVInletOutletScalarBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionOutflowBC(parameters),
    _backflow_value(getFunctor<Real>("backflow_value")),
    _face_flux(getFunctor<Real>("face_flux"))
{
}

const ElemInfo &
LinearFVInletOutletScalarBC::fluidElemInfo() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR
             ? *_current_face_info->neighborInfo()
             : *_current_face_info->elemInfo();
}

bool
LinearFVInletOutletScalarBC::isBackflow() const
{
  return outwardFaceFlux() < 0.0;
}

Real
LinearFVInletOutletScalarBC::outwardFaceFlux() const
{
  const auto state = determineState();
  const Real boundary_normal_multiplier =
      _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? -1.0 : 1.0;
  return boundary_normal_multiplier *
         _face_flux(functorFaceArg(_face_flux, *_current_face_info), state);
}

Real
LinearFVInletOutletScalarBC::computeBackflowBoundaryValue() const
{
  return _backflow_value(functorFaceArg(_backflow_value, *_current_face_info), determineState());
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValue() const
{
  return isBackflow() ? computeBackflowBoundaryValue()
                      : LinearFVAdvectionDiffusionOutflowBC::computeBoundaryValue();
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValue(const bool backflow) const
{
  return backflow ? computeBackflowBoundaryValue()
                  : LinearFVAdvectionDiffusionOutflowBC::computeBoundaryValue();
}

Real
LinearFVInletOutletScalarBC::computeBoundaryNormalGradient() const
{
  if (!isBackflow())
    return LinearFVAdvectionDiffusionOutflowBC::computeBoundaryNormalGradient();

  const auto & elem_info = fluidElemInfo();
  const Real distance = computeCellToFaceDistance();
  return (computeBackflowBoundaryValue() - _var.getElemValue(elem_info, determineState())) /
         distance;
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValueMatrixContribution() const
{
  return isBackflow()
             ? computeBackflowBoundaryValueMatrixContribution()
             : LinearFVAdvectionDiffusionOutflowBC::computeBoundaryValueMatrixContribution();
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValueRHSContribution() const
{
  if (!isBackflow())
    return LinearFVAdvectionDiffusionOutflowBC::computeBoundaryValueRHSContribution();

  const auto & elem_info = fluidElemInfo();
  return computeBackflowBoundaryValue() - computeBackflowBoundaryValueMatrixContribution() *
                                              _var.getElemValue(elem_info, determineState());
}

Real
LinearFVInletOutletScalarBC::computeBoundaryGradientMatrixContribution() const
{
  return isBackflow()
             ? (1.0 - computeBackflowBoundaryValueMatrixContribution()) /
                   computeCellToFaceDistance()
             : LinearFVAdvectionDiffusionOutflowBC::computeBoundaryGradientMatrixContribution();
}

Real
LinearFVInletOutletScalarBC::computeBoundaryGradientRHSContribution() const
{
  return isBackflow()
             ? computeBoundaryValueRHSContribution() / computeCellToFaceDistance()
             : LinearFVAdvectionDiffusionOutflowBC::computeBoundaryGradientRHSContribution();
}
