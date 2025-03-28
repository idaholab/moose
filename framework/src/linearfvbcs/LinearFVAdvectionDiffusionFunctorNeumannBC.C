//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionFunctorNeumannBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionFunctorNeumannBC);

InputParameters
LinearFVAdvectionDiffusionFunctorNeumannBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a fixed gradient BC which can be used for the assembly of linear "
      "finite volume system and whose normal face gradient values are determined "
      "using a functor. This kernel is only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>(
      "functor", "The gradient value functor for this boundary condition.");
  params.addParam<MooseFunctorName>("diffusion_coeff", 1.0, "The diffusion coefficient.");
  return params;
}

LinearFVAdvectionDiffusionFunctorNeumannBC::LinearFVAdvectionDiffusionFunctorNeumannBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _functor(getFunctor<Real>("functor")),
    _diffusion_coeff(getFunctor<Real>("diffusion_coeff"))
{
}

Real
LinearFVAdvectionDiffusionFunctorNeumannBC::computeBoundaryValue() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  // For non-orthogonal meshes we compute an extra correction vector to increase order accuracy
  // correction_vector is a vector orthogonal to the boundary normal
  const auto correction_vector =
      (d_cf - (d_cf * _current_face_info->normal()) * _current_face_info->normal());
  return raw_value(_var(elem_arg, determineState())) +
         _functor(singleSidedFaceArg(_current_face_info), determineState()) /
             _diffusion_coeff(face_arg, determineState()) * distance +
         _var.gradSln(*_current_face_info->elemInfo()) * correction_vector;
}

Real
LinearFVAdvectionDiffusionFunctorNeumannBC::computeBoundaryNormalGradient() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  return _functor(singleSidedFaceArg(_current_face_info), determineState()) /
         _diffusion_coeff(face_arg, determineState());
}

Real
LinearFVAdvectionDiffusionFunctorNeumannBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVAdvectionDiffusionFunctorNeumannBC::computeBoundaryValueRHSContribution() const
{
  const auto face_arg = makeCDFace(*_current_face_info);
  // Fetch the boundary value from the provided functor.
  const Real distance = computeCellToFaceDistance();
  const auto d_cf = computeCellToFaceVector();
  // For non-orthogonal meshes we compute an extra correction vector to increase order accuracy
  // correction_vector is a vector orthogonal to the boundary normal
  const auto correction_vector =
      (d_cf - (d_cf * _current_face_info->normal()) * _current_face_info->normal());
  return +_functor(singleSidedFaceArg(_current_face_info), determineState()) /
             _diffusion_coeff(face_arg, determineState()) * distance +
         _var.gradSln(*_current_face_info->elemInfo()) * correction_vector;
}

Real
LinearFVAdvectionDiffusionFunctorNeumannBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVAdvectionDiffusionFunctorNeumannBC::computeBoundaryGradientRHSContribution() const
{
  return _functor(singleSidedFaceArg(_current_face_info), determineState());
}
