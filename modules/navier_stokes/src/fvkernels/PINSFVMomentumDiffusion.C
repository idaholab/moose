//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumDiffusion.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumDiffusion);

InputParameters
PINSFVMomentumDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Viscous diffusion term, div(mu grad(u_d / eps)), in the porous media "
                             "incompressible Navier-Stokes momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  params.addRequiredParam<MaterialPropertyName>("mu", "viscosity");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  params.addParam<bool>(
      "smooth_porosity", false, "Whether to include the diffusion porosity gradient term");
  params.addParam<MaterialPropertyName>("vel", "The superficial velocity as a material property");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumDiffusion::PINSFVMomentumDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _mu(getFunctorMaterialProperty<ADReal>("mu")),
    _eps(getFunctor<MooseVariableFVReal>("porosity", 0)),
    _index(getParam<MooseEnum>("momentum_component")),
    _vel(isParamValid("vel") ? &getFunctorMaterialProperty<ADRealVectorValue>("vel") : nullptr),
    _eps_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar("porosity", 0))),
    _smooth_porosity(getParam<bool>("smooth_porosity")),
    _cd_limiter()
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumDiffusion may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");

  // Check that the parameters required for the porosity gradient term are set by the user
  if (_smooth_porosity &&
      (!parameters().isParamSetByUser("momentum_component") || !isParamValid("vel")))
    paramError("smooth_porosity",
               "The porosity gradient diffusion term requires specifying "
               "both the momentum component and a superficial velocity material property.");
}

ADReal
PINSFVMomentumDiffusion::computeQpResidual()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  // Compute the diffusion driven by the velocity gradient

  const auto mu_face = _mu(std::make_tuple(_face_info, &_cd_limiter, true));
  const auto eps_face = _eps(std::make_tuple(_face_info, &_cd_limiter, true));

  // Compute face superficial velocity gradient
  auto dudn = gradUDotNormal();

  // First term of residual
  ADReal residual = mu_face / eps_face * dudn;

  if (_smooth_porosity)
  {
    // Get the face porosity gradient separately
    const auto & grad_eps_face = MetaPhysicL::raw_value(_eps_var->adGradSln(*_face_info));

    ADRealVectorValue term_face = mu_face / (eps_face * eps_face) * grad_eps_face;
    const auto vel_face = (*_vel)(std::make_tuple(_face_info, &_cd_limiter, true));

    for (int i = 0; i < LIBMESH_DIM; i++)
      term_face(i) *= vel_face(i);

    residual -= term_face * _normal;
  }
  return -residual;
#else
  return 0;
#endif
}
