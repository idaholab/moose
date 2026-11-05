//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PMomentumDriftFlux.h"
#include "INSFVRhieChowInterpolator.h"
#include "NS.h"
#include "SystemBase.h"
#include "RelationshipManager.h"
#include "Factory.h"

registerMooseObject("NavierStokesApp", WCNSFV2PMomentumDriftFlux);

InputParameters
WCNSFV2PMomentumDriftFlux::validParams()
{
  auto params = INSFVFluxKernel::validParams();
  params.addClassDescription(
      "Implements the diffusion (drift) stress of the two-phase mixture model, "
      "div(beta_d beta_c / rho_m * u_slip (x) u_slip), on the left hand side of the mixture "
      "momentum equation.");
  params.addRequiredParam<MooseFunctorName>("u_slip", "The slip velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("rho_d", "Dispersed phase density.");
  params.addRequiredParam<MooseFunctorName>("rho_c", "Continuous phase density.");
  params.addParam<MooseFunctorName>("fd", 0.0, "Fraction dispersed phase.");

  params.renameParam("fd", "fraction_dispersed", "");

  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("density_interp_method",
                             coeff_interp_method,
                             "Switch that can select face interpolation method for the density.");

  return params;
}

WCNSFV2PMomentumDriftFlux::WCNSFV2PMomentumDriftFlux(const InputParameters & params)
  : INSFVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _rho_d(getFunctor<ADReal>("rho_d")),
    _rho_c(getFunctor<ADReal>("rho_c")),
    _f_d(getFunctor<ADReal>("fd")),
    _u_slip(getFunctor<ADReal>("u_slip")),
    _v_slip(isParamValid("v_slip") ? &getFunctor<ADReal>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<ADReal>("w_slip") : nullptr),
    _density_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("density_interp_method")))
{
  if (_dim >= 2 && !_v_slip)
    mooseError("In two or more dimensions, the v_slip velocity must be supplied using the 'v_slip' "
               "parameter");
  if (_dim >= 3 && !_w_slip)
    mooseError(
        "In three dimensions, the w_slip velocity must be supplied using the 'w_slip' parameter");

  // Phase fraction could be a nonlinear variable
  const auto & fraction_name = getParam<MooseFunctorName>("fraction_dispersed");
  if (isParamValid("fraction_dispersed") && _fe_problem.hasVariable(fraction_name))
    addMooseVariableDependency(&_fe_problem.getVariable(_tid, fraction_name));
}

ADReal
WCNSFV2PMomentumDriftFlux::computeStrongResidual(const bool populate_a_coeffs)
{
  _normal = _face_info->normal();
  const auto state = determineState();

  Moose::FaceArg face_arg;
  if (onBoundary(*_face_info))
    face_arg = singleSidedFaceArg();
  else
    face_arg = makeCDFace(*_face_info);

  ADRealVectorValue u_slip_vel_vec;
  if (_dim == 1)
    u_slip_vel_vec = ADRealVectorValue(_u_slip(face_arg, state), 0.0, 0.0);
  else if (_dim == 2)
    u_slip_vel_vec = ADRealVectorValue(_u_slip(face_arg, state), (*_v_slip)(face_arg, state), 0.0);
  else
    u_slip_vel_vec = ADRealVectorValue(
        _u_slip(face_arg, state), (*_v_slip)(face_arg, state), (*_w_slip)(face_arg, state));

  const auto uslipdotn = _normal * u_slip_vel_vec;

  // The exact diffusion stress coefficient, beta_d beta_c / rho_m, evaluated on the face. It
  // vanishes wherever either phase is absent and the harmonic mean is not defined there, so those
  // faces fall back to the arithmetic average.
  ADReal face_coefficient;
  if (onBoundary(*_face_info))
    face_coefficient = diffusionStressCoefficient(makeCDFace(*_face_info), state);
  else
  {
    const auto elem_coefficient = diffusionStressCoefficient(elemArg(), state);
    const auto neighbor_coefficient = diffusionStressCoefficient(neighborArg(), state);
    const auto interp_method = (elem_coefficient > 0.0 && neighbor_coefficient > 0.0)
                                   ? _density_interp_method
                                   : Moose::FV::InterpMethod::Average;
    Moose::FV::interpolate(
        interp_method, face_coefficient, elem_coefficient, neighbor_coefficient, *_face_info, true);
  }

  if (populate_a_coeffs)
  {
    if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
        _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    {
      const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
      if (_index == 0)
        _ae = (uslipdotn * _u_slip(elemArg(), state)).derivatives()[dof_number];
      else if (_index == 1)
        _ae = (uslipdotn * (*_v_slip)(elemArg(), state)).derivatives()[dof_number];
      else
        _ae = (uslipdotn * (*_w_slip)(elemArg(), state)).derivatives()[dof_number];
      _ae *= face_coefficient;
    }
    if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
        _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    {
      const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
      if (_index == 0)
        _an = (uslipdotn * _u_slip(neighborArg(), state)).derivatives()[dof_number];
      else if (_index == 1)
        _an = (uslipdotn * (*_v_slip)(neighborArg(), state)).derivatives()[dof_number];
      else
        _an = (uslipdotn * (*_w_slip)(neighborArg(), state)).derivatives()[dof_number];
      _an *= -face_coefficient;
    }
  }

  // Summing the phase momentum equations puts +div(sum_k a_k rho_k u_Mk u_Mk) on the left hand
  // side, hence the positive sign
  return face_coefficient * uslipdotn * u_slip_vel_vec(_index);
}

void
WCNSFV2PMomentumDriftFlux::gatherRCData(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));

  addResidualAndJacobian(computeStrongResidual(true) * (fi.faceArea() * fi.faceCoord()));

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(&fi.elem(), _index, _ae * (fi.faceArea() * fi.faceCoord()));
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(fi.neighborPtr(), _index, _an * (fi.faceArea() * fi.faceCoord()));
}

ADReal
WCNSFV2PMomentumDriftFlux::computeSegregatedContribution()
{
  return computeStrongResidual(false);
}
