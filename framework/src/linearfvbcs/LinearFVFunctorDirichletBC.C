//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVFunctorDirichletBC.h"

registerMooseObject("MooseApp", LinearFVFunctorDirichletBC);

InputParameters
LinearFVFunctorDirichletBC::validParams()
{
  InputParameters params = LinearFVBoundaryCondition::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "The functor for this boundary condition.");
  return params;
}

LinearFVFunctorDirichletBC::LinearFVFunctorDirichletBC(const InputParameters & parameters)
  : LinearFVBoundaryCondition(parameters), _functor(getFunctor<Real>("functor"))
{
}

Real
LinearFVFunctorDirichletBC::computeBoundaryValue(const FaceInfo * const face_info)
{
  return _functor(singleSidedFaceArg(face_info), determineState());
}

Real
LinearFVFunctorDirichletBC::computeCellToFaceDistance(const FaceInfo * const face_info) const
{
  const auto is_on_mesh_boundary = face_info->neighborPtr();
  const auto defined_on_elem =
      is_on_mesh_boundary
          ? true
          : (face_info->faceType(std::make_pair(_var->number(), _var->sys().number())) ==
             FaceInfo::VarFaceNeighbors::ELEM);
  if (is_on_mesh_boundary)
    return face_info->dCNMag();
  else
    return (face_info->faceCentroid() -
            (defined_on_elem ? face_info->elemCentroid() : face_info->neighborCentroid()))
        .norm();
}

Real
LinearFVFunctorDirichletBC::computeBoundaryNormalGradient(const FaceInfo * const face_info)
{
  const auto defined_on_elem =
      face_info->faceType(std::make_pair(_var->number(), _var->sys().number())) ==
      FaceInfo::VarFaceNeighbors::ELEM;

  const auto elem_arg =
      makeElemArg(defined_on_elem ? face_info->elemPtr() : face_info->neighborPtr());
  Real distance = computeCellToFaceDistance(face_info);
  return (_functor(singleSidedFaceArg(face_info), determineState()) -
          raw_value((*_var)(elem_arg, determineState()))) /
         distance;
}

Real
LinearFVFunctorDirichletBC::computeBoundaryValueMatrixContribution(
    const FaceInfo * const /*face_info*/) const
{
  return 0.0;
}
Real
LinearFVFunctorDirichletBC::computeBoundaryValueRHSContribution(
    const FaceInfo * const face_info) const
{
  return _functor(singleSidedFaceArg(face_info), determineState());
}

Real
LinearFVFunctorDirichletBC::computeBoundaryGradientMatrixContribution(
    const FaceInfo * const face_info) const
{
  Real distance = computeCellToFaceDistance(face_info);
  return face_info->faceArea() / distance;
}

Real
LinearFVFunctorDirichletBC::computeBoundaryGradientRHSContribution(
    const FaceInfo * const face_info) const
{
  Real distance = computeCellToFaceDistance(face_info);
  return _functor(singleSidedFaceArg(face_info), determineState()) * face_info->faceArea() /
         distance;
}
