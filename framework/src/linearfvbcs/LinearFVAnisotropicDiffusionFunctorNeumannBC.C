//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAnisotropicDiffusionFunctorNeumannBC.h"

registerMooseObject("MooseApp", LinearFVAnisotropicDiffusionFunctorNeumannBC);

InputParameters
LinearFVAnisotropicDiffusionFunctorNeumannBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorNeumannBC::validParams();
  params.suppressParameter<MooseFunctorName>("diffusion_coeff");
  params.addRequiredParam<MooseFunctorName>("diffusion_tensor",
                                            "Functor describing a diagonal diffusion tensor.");
  params.addClassDescription(
      "Adds a prescribed anisotropic diffusive flux boundary condition to a linear finite "
      "volume system and reconstructs the boundary value using a diagonal diffusion tensor.");
  return params;
}

LinearFVAnisotropicDiffusionFunctorNeumannBC::LinearFVAnisotropicDiffusionFunctorNeumannBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorNeumannBC(parameters),
    _diffusion_tensor(getFunctor<RealVectorValue>("diffusion_tensor"))
{
}

Real
LinearFVAnisotropicDiffusionFunctorNeumannBC::computeBoundaryValue() const
{
  const auto state = determineState();
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  const auto boundary_normal =
      (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0) *
      _current_face_info->normal();
  const auto cell_to_face = computeCellToFaceVector();
  const auto tangential_cell_to_face =
      cell_to_face - (cell_to_face * boundary_normal) * boundary_normal;

  return _var.getElemValue(*elem_info, state) +
         computeBoundaryNormalGradient() * computeCellToFaceDistance() +
         _var.gradSln(*elem_info, state) * tangential_cell_to_face;
}

Real
LinearFVAnisotropicDiffusionFunctorNeumannBC::computeBoundaryNormalGradient() const
{
  const auto state = determineState();
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  const auto boundary_normal =
      (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0) *
      _current_face_info->normal();
  const auto diffusion_tensor = _diffusion_tensor(singleSidedFaceArg(_current_face_info), state);

  RealVectorValue normal_scaled_diffusion;
  for (const auto i : make_range(Moose::dim))
    normal_scaled_diffusion(i) = boundary_normal(i) * diffusion_tensor(i);

  const Real normal_diffusion = normal_scaled_diffusion * boundary_normal;
  if (normal_diffusion <= 0.0)
    mooseError("The boundary-normal diffusion coefficient must be positive, but its value is ",
               normal_diffusion,
               ".");

  const auto tangential_diffusion = normal_scaled_diffusion - normal_diffusion * boundary_normal;
  const Real tangential_flux = tangential_diffusion * _var.gradSln(*elem_info, state);

  return (_functor(singleSidedFaceArg(_current_face_info), state) - tangential_flux) /
         normal_diffusion;
}

Real
LinearFVAnisotropicDiffusionFunctorNeumannBC::computeBoundaryValueRHSContribution() const
{
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();
  return computeBoundaryValue() - _var.getElemValue(*elem_info, determineState());
}
