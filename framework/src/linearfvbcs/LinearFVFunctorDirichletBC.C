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
  params.addClassDescription("Adds a dirichlet BC which can be used for the assembly of non-Newton "
                             "systems and whose face values are determined using a functor.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor for this boundary condition.");
  return params;
}

LinearFVFunctorDirichletBC::LinearFVFunctorDirichletBC(const InputParameters & parameters)
  : LinearFVBoundaryCondition(parameters), _functor(getFunctor<Real>("functor"))
{
}

Real
LinearFVFunctorDirichletBC::computeBoundaryValue()
{
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVFunctorDirichletBC::computeCellToFaceDistance() const
{
  const auto is_on_mesh_boundary = !_current_face_info->neighborPtr();
  const auto defined_on_elem =
      is_on_mesh_boundary ? true : (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM);
  if (is_on_mesh_boundary)
    return std::abs(_current_face_info->dCN() * _current_face_info->normal());
  else
    return std::abs((_current_face_info->faceCentroid() -
                     (defined_on_elem ? _current_face_info->elemCentroid()
                                      : _current_face_info->neighborCentroid())) *
                    _current_face_info->normal());
}

Real
LinearFVFunctorDirichletBC::computeBoundaryNormalGradient()
{
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();
  return (_functor(singleSidedFaceArg(_current_face_info), determineState()) -
          raw_value((*_var)(elem_arg, determineState()))) /
         distance;
}

Real
LinearFVFunctorDirichletBC::computeBoundaryValueMatrixContribution() const
{
  return 0.0;
}
Real
LinearFVFunctorDirichletBC::computeBoundaryValueRHSContribution() const
{
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVFunctorDirichletBC::computeBoundaryGradientMatrixContribution() const
{
  const Real distance = computeCellToFaceDistance();
  return _current_face_info->faceArea() / distance;
}

Real
LinearFVFunctorDirichletBC::computeBoundaryGradientRHSContribution() const
{
  const Real distance = computeCellToFaceDistance();
  return _functor(singleSidedFaceArg(_current_face_info), determineState()) *
         _current_face_info->faceArea() / distance;
}
