//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAnisotropicDiffusion.h"
#include "RelationshipManager.h"

registerMooseObject("MooseApp", FVAnisotropicDiffusion);

InputParameters
FVAnisotropicDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Computes residual for anisotropic diffusion operator for finite volume method.");
  params.addRequiredParam<MooseFunctorName>("coeff", "diffusion coefficient");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVAnisotropicDiffusion::FVAnisotropicDiffusion(const InputParameters & params)
  : FVFluxKernel(params), _coeff(getFunctor<ADRealVectorValue>("coeff"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("FVAnisotropicDiffusion is not supported by local AD indexing. In order to use this "
             "object, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'. Note that global indexing is now the default "
             "configuration for AD indexing type.");
#endif

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
FVAnisotropicDiffusion::computeQpResidual()
{
  ADReal r = 0;
  const auto face_elem = elemFromFace();
  const auto face_neighbor = neighborFromFace();
  const auto & grad_T = _var.adGradSln(*_face_info);
  for (std::size_t i = 0; i < LIBMESH_DIM; i++)
  {
    ADReal k_face_inv;
    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           k_face_inv,
                           1.0 / _coeff(face_elem)(i),
                           1.0 / _coeff(face_neighbor)(i),
                           *_face_info,
                           true);
    r += _normal(i) * grad_T(i) / k_face_inv;
  }
  return -r;
}
