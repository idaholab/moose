//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionFunctorRobinBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionFunctorRobinBC);

InputParameters
LinearFVAdvectionDiffusionFunctorRobinBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds a Robin BC of the form alpha* grad_phi*n + beta*phi = gamma, "
                             "which can be used for the assembly of linear "
                             "finite volume system and whose face values are determined using "
                             "three functors. This kernel is "
                             "only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>(
      "alpha", "The functor which is the coefficient of the normal gradient term.");
  params.addRequiredParam<MooseFunctorName>(
      "beta", "The functor which is the coefficient of the scalar term.");
  params.addRequiredParam<MooseFunctorName>(
      "gamma", "The functor which is the constant term on the RHS of the Robin BC expression.");
  return params;
}

LinearFVAdvectionDiffusionFunctorRobinBC::LinearFVAdvectionDiffusionFunctorRobinBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _alpha(getFunctor<Real>("alpha")),
    _beta(getFunctor<Real>("beta")),
    _gamma(getFunctor<Real>("gamma"))
{
  _var.computeCellGradients();

  if (_alpha.isConstant())
  {
    // We check if we can parse the value to a number and if yes, we throw an error if it is 0
    std::istringstream ss(getParam<MooseFunctorName>("alpha"));
    Real real_value;
    if (ss >> real_value && ss.eof())
      if (MooseUtils::isZero(real_value))
        paramError("alpha",
                   "This value shall not be 0. Use a Dirichlet boundary condition instead!");
  }
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValue() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto beta = _beta(face, state);
  const auto gamma = _gamma(face, state);

  const auto phi = _var.getElemValue(*elem_info, state);
  const auto grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat);
  return ((alpha * phi) + (alpha * grad_phi * vc) + (gamma * projection)) /
         (alpha + (beta * projection));
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryNormalGradient() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = _alpha(face, state);
  mooseAssert(!MooseUtils::isZero(alpha), "Alpha should not be 0!");
  const auto beta = _beta(face, state);
  const auto gamma = _gamma(face, state);
  return (gamma - beta * computeBoundaryValue()) / alpha;
}

// implicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = _alpha(face, state);
  const auto beta = _beta(face, state);
  const auto & nhat = _current_face_info->normal();

  return alpha / (alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto alpha = _alpha(face, state);
  const auto beta = _beta(face, state);
  const auto gamma = _gamma(face, state);
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma * projection / (alpha + (beta * projection))) +
         (alpha * grad_phi * vc / (alpha + (beta * projection)));
}

// implicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto beta = _beta(face, state);
  const auto & nhat = _current_face_info->normal();

  return beta / (alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientRHSContribution() const
{
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto alpha = _alpha(face, state);
  const auto beta = _beta(face, state);
  const auto gamma = _gamma(face, state);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma / alpha) + (-beta * gamma * projection / alpha / (alpha + (beta * projection))) +
         (-beta * grad_phi * vc / (alpha + (beta * projection)));
}
