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
LinearFVFunctorDirichletBC::computeBoundaryValue() const
{
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVFunctorDirichletBC::computeBoundaryNormalGradient() const
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
  // Ths will not contribute to the matrix from the value considering that
  // the value is independent of the solution.
  return 0.0;
}

Real
LinearFVFunctorDirichletBC::computeBoundaryValueRHSContribution() const
{
  // Fetch the boundary value from the provided functor.
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVFunctorDirichletBC::computeBoundaryGradientMatrixContribution() const
{
  // The implicit term from the central difference approximation of the normal
  // gradient.
  return 1.0 / computeCellToFaceDistance();
}

Real
LinearFVFunctorDirichletBC::computeBoundaryGradientRHSContribution() const
{
  // The boundary term from the central difference approximation of the
  // normal gradient.
  return _functor(singleSidedFaceArg(_current_face_info), determineState()) /
         computeCellToFaceDistance();
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
