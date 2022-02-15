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
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVMomentumDiffusion::INSFVMomentumDiffusion(const InputParameters & params)
  : INSFVFluxKernel(params), _mu(getFunctor<ADReal>(NS::mu))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
  {
    auto & factory = _app.getFactory();

    auto rm_params = factory.getValidParams("ElementSideNeighborLayers");

    rm_params.set<std::string>("for_whom") = name();
    rm_params.set<MooseMesh *>("mesh") = &const_cast<MooseMesh &>(_mesh);
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
        Moose::RelationshipManagerType::COUPLING;
    FVKernel::setRMParams(
        _pars, rm_params, std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
    mooseAssert(rm_params.areAllRequiredParamsValid(),
                "All relationship manager parameters should be valid.");

    auto rm_obj = factory.create<RelationshipManager>(
        "ElementSideNeighborLayers", name() + "_skew_correction", rm_params);

    // Delete the resources created on behalf of the RM if it ends up not being added to the
    // App.
    if (!_app.addRelationshipManager(rm_obj))
      factory.releaseSharedObjects(*rm_obj);
  }
}

ADReal
INSFVMomentumDiffusion::computeStrongResidual()
{
  const auto face = Moose::FV::makeCDFace(*_face_info, faceArgSubdomains());
  const auto dudn = gradUDotNormal();
  const auto face_mu = _mu(face);

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

  processResidual(computeStrongResidual() * (fi.faceArea() * fi.faceCoord()));

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(&fi.elem(), _index, _ae * (fi.faceArea() * fi.faceCoord()));
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(fi.neighborPtr(), _index, _an * (fi.faceArea() * fi.faceCoord()));
}
