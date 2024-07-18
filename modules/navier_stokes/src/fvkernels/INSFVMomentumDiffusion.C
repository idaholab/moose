//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumDiffusion.h"
#include "INSFVRhieChowInterpolator.h"
#include "NS.h"
#include "NavierStokesMethods.h"
#include "SystemBase.h"
#include "NonlinearSystemBase.h"
#include "RelationshipManager.h"
#include "Factory.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVMomentumDiffusion);

InputParameters
INSFVMomentumDiffusion::validParams()
{
  auto params = INSFVFluxKernel::validParams();
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The viscosity");
  params.addClassDescription(
      "Implements the Laplace form of the viscous stress in the Navier-Stokes equation.");

  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("mu_interp_method",
                             coeff_interp_method,
                             "Switch that can select face interpolation method for the viscosity.");

  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<bool>(
      "complete_expansion",
      false,
      "Boolean parameter to use complete momentum expansion is the diffusion term.");
  params.addParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addParam<bool>(
      "limit_interpolation", false, "Flag to limit interpolation to positive values.");
  params.addParam<bool>("newton_solve", false, "Whether a Newton nonlinear solve is being used");
  params.addParamNamesToGroup("newton_solve", "Advanced");
  return params;
}

INSFVMomentumDiffusion::INSFVMomentumDiffusion(const InputParameters & params)
  : INSFVFluxKernel(params),
    SolutionInvalidInterface(this),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("mu_interp_method"))),
    _u_var(params.isParamValid("u") ? &getFunctor<ADReal>("u") : nullptr),
    _v_var(params.isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w_var(params.isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _complete_expansion(getParam<bool>("complete_expansion")),
    _limit_interpolation(getParam<bool>("limit_interpolation")),
    _dim(_subproblem.mesh().dimension()),
    _newton_solve(getParam<bool>("newton_solve"))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));

  if (_complete_expansion && !_u_var)
    paramError("u", "The u velocity must be defined when 'complete_expansion=true'.");

  if (_complete_expansion && _dim >= 2 && !_v_var)
    paramError("v",
               "The v velocity must be defined when 'complete_expansion=true'"
               "and problem dimension is larger or equal to 2.");

  if (_complete_expansion && _dim >= 3 && !_w_var)
    paramError("w",
               "The w velocity must be defined when 'complete_expansion=true'"
               "and problem dimension is larger or equal to three.");
}

ADReal
INSFVMomentumDiffusion::computeStrongResidual(const bool populate_a_coeffs)
{
  const Moose::StateArg state = determineState();
  const auto dudn = gradUDotNormal(state);
  ADReal face_mu;

  if (onBoundary(*_face_info))
    face_mu = _mu(makeCDFace(*_face_info), state);
  else
    Moose::FV::interpolate(_mu_interp_method,
                           face_mu,
                           _mu(elemArg(), state),
                           _mu(neighborArg(), state),
                           *_face_info,
                           true);

  // Protecting from negative viscosity at interpolation
  // to preserve convergence
  if (face_mu < 0.0)
  {
    if (!(_limit_interpolation))
    {
      mooseDoOnce(mooseWarning(
          "Negative face viscosity has been encountered. Value ",
          raw_value(face_mu),
          " at ",
          _face_info->faceCentroid(),
          " limiting it to 0!\nFurther warnings for this issue will be silenced, but the "
          "occurrences will be recorded through the solution invalidity interface."));
      flagInvalidSolution("Negative face dynamic viscosity has been encountered.");
    }
    // Keep face_mu here for sparsity pattern detection
    face_mu = 0 * face_mu;
  }

  if (populate_a_coeffs)
  {
    if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
        _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    {
      const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
      // A gradient is a linear combination of degrees of freedom so it's safe to straight-up index
      // into the derivatives vector at the dof we care about
      _ae = dudn.derivatives()[dof_number];
      _ae *= -face_mu;
    }
    if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
        _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    {
      const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
      _an = dudn.derivatives()[dof_number];
      _an *= face_mu;
    }
  }

  ADReal dudn_transpose = 0.0;
  if (_complete_expansion)
  {
    // Computing the gradient from coupled variables
    // Normally, we can do this with `_var.gradient(face, state)` but we will need the transpose
    // gradient. So, we compute all at once
    Moose::FaceArg face;
    const bool skewness_correction =
        (_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);
    if (onBoundary(*_face_info))
      face = singleSidedFaceArg();
    else
      face = makeCDFace(*_face_info, skewness_correction);

    ADRealTensorValue gradient;
    if (_dim == 1)
    {
      const auto & grad_u = _u_var->gradient(face, state);
      gradient = ADRealTensorValue(grad_u, ADRealVectorValue(0, 0, 0), ADRealVectorValue(0, 0, 0));
    }
    else if (_dim == 2)
    {
      const auto & grad_u = _u_var->gradient(face, state);
      const auto & grad_v = _v_var->gradient(face, state);
      gradient = ADRealTensorValue(grad_u, grad_v, ADRealVectorValue(0, 0, 0));
    }
    else // if (_dim == 3)
    {
      const auto & grad_u = _u_var->gradient(face, state);
      const auto & grad_v = _v_var->gradient(face, state);
      const auto & grad_w = _w_var->gradient(face, state);
      gradient = ADRealTensorValue(grad_u, grad_v, grad_w);
    }

    // Getting transpose of the gradient matrix
    const auto gradient_transpose = gradient.transpose();

    dudn_transpose += gradient_transpose.row(_index) * _face_info->normal();
  }

  return -face_mu * (dudn + dudn_transpose);
}

void
INSFVMomentumDiffusion::gatherRCData(const FaceInfo & fi)
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
INSFVMomentumDiffusion::computeSegregatedContribution()
{
  return computeStrongResidual(false);
}
