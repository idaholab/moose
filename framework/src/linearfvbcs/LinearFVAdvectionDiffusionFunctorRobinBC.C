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
  params.addClassDescription(
      "Adds a Robin BC of the form alpha* grad_phi*n + beta*phi = gamma, "
      "which can be used for the assembly of linear "
      "finite volume system and whose face values are determined using three functors. This kernel is "
      "only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>("alpha", "The functor which is the coefficient of the normal gradient term.");
  params.addRequiredParam<MooseFunctorName>("beta", "The functor which is the coefficient of the scalar term.");
  params.addRequiredParam<MooseFunctorName>("gamma", "The functor which is the constant term on the RHS of the Robin BC expression.");
  return params;
}

LinearFVAdvectionDiffusionFunctorRobinBC::LinearFVAdvectionDiffusionFunctorRobinBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters), 
    _alpha(getFunctor<Real>("alpha")),
    _beta(getFunctor<Real>("beta" )),
    _gamma(getFunctor<Real>("gamma"))
{
// TODO: PG will attempt better error-handling during the PR review.

//  if(_alpha( singleSidedFaceArg(_current_face_info), determineState()) == 0)
//    mooseError("Setting alpha, the coefficient of the gradient term, to zero in the Robin"
//               "boundary condition results in NaNs. Consider using"
//               "LinearFVAdvectionDiffusionFunctorDirichletBC instead.");

  _var.computeCellGradients();
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValue() const
{

  const auto face = singleSidedFaceArg(_current_face_info);
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto beta  = _beta(face, state);
  const auto gamma = _gamma(face, state);

  const auto phi = raw_value(_var(elem_arg, state));
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const auto nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto vc = d_cf - (d_cf * nhat * nhat);
  return ( (alpha * phi) 
           + ( alpha * grad_phi * vc)
           + (gamma * d_cf * nhat)
         ) / ( alpha + (beta * computeCellToFaceVector() * nhat) );
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryNormalGradient() const
{

  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());

  const auto alpha = _alpha(face, state);
  const auto beta  = _beta(face, state);
  const auto gamma = _gamma(face, state);
  const auto phi   = raw_value(_var(elem_arg, state));

  return (gamma - beta * phi) / alpha;
}

// implicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueMatrixContribution() const
{
  const auto alpha = _alpha( singleSidedFaceArg(_current_face_info),  determineState());
  const auto beta  = _beta( singleSidedFaceArg(_current_face_info),  determineState());
  const auto nhat = _current_face_info->normal();

  return alpha/( alpha + (beta * computeCellToFaceVector() * nhat) );
}

// explicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto beta  = _beta(face, state);
  const auto gamma = _gamma(face, state);
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const auto nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto vc = d_cf - ((d_cf * nhat) * nhat); //correction vector for non-orthogonal cells

  return (gamma * d_cf * nhat /( alpha + (beta * computeCellToFaceVector() * nhat) ) ) +
          (alpha * grad_phi * vc /( alpha + (beta * computeCellToFaceVector() * nhat) ) );
}

// implicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientMatrixContribution() const
{
  const auto alpha = _alpha( singleSidedFaceArg(_current_face_info),  determineState());
  const auto beta  = _beta( singleSidedFaceArg(_current_face_info),  determineState());
  const auto nhat = _current_face_info->normal();

  return beta/( alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const auto alpha = _alpha(face, state);
  const auto beta  =  _beta(face, state);
  const auto gamma = _gamma(face, state);

  const auto nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto vc = d_cf - ((d_cf * nhat) * nhat); //correction vector for non-orthogonal cells

  return ( gamma/alpha) +
          ( -beta *gamma * nhat * d_cf / alpha / ( alpha + (beta * computeCellToFaceVector() * nhat) ) ) +
           ( -beta * grad_phi * vc / ( alpha + (beta * computeCellToFaceVector() * nhat) ) );
}
