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
#include "SystemBase.h"
#include "RelationshipManager.h"
#include "Factory.h"

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
  return params;
}

INSFVMomentumDiffusion::INSFVMomentumDiffusion(const InputParameters & params)
  : INSFVFluxKernel(params),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_interp_method(Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("mu_interp_method")))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
}

ADReal
INSFVMomentumDiffusion::computeStrongResidual()
{
  const auto state = determineState();
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

  return -face_mu * dudn;
}

void
INSFVMomentumDiffusion::gatherRCData(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  processResidualAndJacobian(computeStrongResidual() * (fi.faceArea() * fi.faceCoord()));

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(&fi.elem(), _index, _ae * (fi.faceArea() * fi.faceCoord()));
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(fi.neighborPtr(), _index, _an * (fi.faceArea() * fi.faceCoord()));
}
