//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionFunctorDirichletBC);

InputParameters
LinearFVAdvectionDiffusionFunctorDirichletBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a dirichlet BC which can be used for the assembly of linear "
      "finite volume system and whose face values are determined using a functor. This kernel is "
      "only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor for this boundary condition.");
  return params;
}

LinearFVAdvectionDiffusionFunctorDirichletBC::LinearFVAdvectionDiffusionFunctorDirichletBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters), _functor(getFunctor<Real>("functor"))
{
}

Real
LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryValue() const
{
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryNormalGradient() const
{
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();
  return (_functor(singleSidedFaceArg(_current_face_info), determineState()) -
          raw_value(_var(elem_arg, determineState()))) /
         distance;
}

Real
LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryValueMatrixContribution() const
{
  // Ths will not contribute to the matrix from the value considering that
  // the value is independent of the solution.
  return 0.0;
}

Real
LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryValueRHSContribution() const
{
  // Fetch the boundary value from the provided functor.
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryGradientMatrixContribution() const
{
  // The implicit term from the central difference approximation of the normal
  // gradient.
  return 1.0 / computeCellToFaceDistance();
}

Real
LinearFVAdvectionDiffusionFunctorDirichletBC::computeBoundaryGradientRHSContribution() const
{
  // The boundary term from the central difference approximation of the
  // normal gradient.
  return _functor(singleSidedFaceArg(_current_face_info), determineState()) /
         computeCellToFaceDistance();
}
