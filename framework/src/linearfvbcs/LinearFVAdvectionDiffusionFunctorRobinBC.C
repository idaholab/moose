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
  params.addRequiredParam<MooseFunctorName>("functor-alpha", "The functor which is the coefficient of the normal gradient term.");
  params.addRequiredParam<MooseFunctorName>("functor-beta", "The functor which is the coefficient of the scalar term.");
  params.addRequiredParam<MooseFunctorName>("functor-gamma", "The functor which is the constant term on the RHS of the Robin BC expression.");
  return params;
}

LinearFVAdvectionDiffusionFunctorRobinBC::LinearFVAdvectionDiffusionFunctorRobinBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters), _functor-alpha(getFunctor<Real>("functor-alpha")),
  : LinearFVAdvectionDiffusionBC(parameters),  _functor-beta(getFunctor<Real>("functor-beta" )),
  : LinearFVAdvectionDiffusionBC(parameters), _functor-gamma(getFunctor<Real>("functor-gamma"))
{
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValue() const
{
  return _functor-alpha(singleSidedFaceArg(_current_face_info), determineState());
  return  _functor-beta(singleSidedFaceArg(_current_face_info), determineState());
  return _functor-gamma(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryNormalGradient() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());

  const Real distance = computeCellToFaceDistance();
  const auto alpha = _functor-alpha(face, state);
  const auto beta  =  _functor-beta(face, state);
  const auto gamma = _functor-gamma(face, state);
  const auto phi_n = raw_value(_var(elem_arg, state));

  return (gamma - beta * phi_n) / alpha;
}

RealVectorValue
LinearFVAdvectionDiffusionFunctorRobinBC::computeFaceTangentVector() const()
{
  // returns distance vector from nearest cell centre to boundary face
  const auto d_cf = computeCellToFaceVector();
  const auto nhat = _current_face_info->normal();

  // tangent along the face 
  return d_cf - (d_cf * nhat) * nhat;
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeRobinDenominatorTerm() const()
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  // returns distance vector from nearest cell centre to boundary face
  const auto d_cf  = computeCellToFaceVector();
  // face normal
  const auto nhat  = _current_face_info->normal();
  // robin BC coefficients
  const auto alpha = _functor-alpha(face, state);
  const auto beta  = _functor-beta(face, state);

  return 1.0 + (beta * d_cf * nhat / alpha);
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0/computeRobinDenominatorTerm();
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _functor-alpha(face, state);
  const auto gamma = _functor-gamma(face, state);
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  // returns distance vector from nearest cell centre to boundary face
  const auto d_cf = computeCellToFaceVector();
  const auto tvec = computeFaceTangentVector();
  const auto nhat = _current_face_info->normal();

  const Real dmag = std::sqrt(d_cf*d_cf);
  const Real dnom = computeRobinDenominatorTerm();

  return ( dmag * (grad_phi * tvec) / dnom) + 
         ( (d_cf*nhat) * gamma / alpha / dnom);
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto beta  =  _functor-beta(face, state);
  const auto alpha = _functor-alpha(face, state);

  return -beta/alpha/computeRobinDenominatorTerm();	
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientRHSContribution() const
{
  // The boundary term from the central difference approximation of the
  // normal gradient.
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const auto alpha = _functor-alpha(face, state);
  const auto beta  =  _functor-beta(face, state);
  const auto gamma = _functor-gamma(face, state);

  const auto d_cf = computeCellToFaceVector();
  const auto tvec = computeFaceTangentVector();
  const auto nhat = _current_face_info->normal();

  const Real dmag = std::sqrt(d_cf*d_cf);
  const Real dnom = computeRobinDenominatorTerm();

  return gamma - (beta/alpha/denom)*
           ( (d_cf * nhat * gamma/alpha) + (dmag*grad_phi*tvec) );
}
