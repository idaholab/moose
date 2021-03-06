//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumDiffusion.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumDiffusion);

InputParameters
PINSFVMomentumDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Viscous diffusion term, div(mu grad(u)), in the porous media "
                             "incompressible Navier-Stokes momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  params.addRequiredParam<MaterialPropertyName>("mu", "viscosity");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<bool>("smooth_porosity", false, "Whether to compute the porosity gradient diffusive term");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumDiffusion::PINSFVMomentumDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
  _mu_elem(isParamValid("mu") ? &getADMaterialProperty<Real>("mu") : nullptr),
  _mu_neighbor(isParamValid("mu") ? &getNeighborADMaterialProperty<Real>("mu") : nullptr),
  _mu_eff_elem(isParamValid("mu_eff") ? &getADMaterialProperty<Real>("mu_eff") : nullptr),
  _mu_eff_neighbor(isParamValid("mu_eff") ? &getNeighborADMaterialProperty<Real>("mu_eff") : nullptr),
  _effective_viscosity(isParamValid("mu_eff")),
  _eps(coupledValue("porosity")),
  _eps_neighbor(coupledNeighborValue("porosity")),
  _index(getParam<MooseEnum>("momentum_component")),
  _vel_elem(getADMaterialProperty<RealVectorValue>(NS::velocity)),
  _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::velocity)),
  _eps_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar("porosity", 0))),
  _smooth_porosity(getParam<bool>("smooth_porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (_smooth_porosity && !params.isParamSetByUser("momentum_component"))
    mooseError("The momentum component parameter is required for modeling the porosity "
               "gradient contribution in the momentum diffusion term.");
  if (_smooth_porosity && isParamValid("mu_eff"))
    paramError("mu_eff", "The porosity gradient term need not be included when using an "
               "effective viscosity.");
  if (isParamValid("mu") == isParamValid("mu_eff"))
    mooseError("Either the viscosity or the effective viscosity must be specified.");
}

ADReal
PINSFVMomentumDiffusion::computeQpResidual()
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#else
  // TODO: Rewrite with fluid velocity once AuxVariables support AD

  // Compute the diffusion driven by the velocity gradient
  // Interpolate viscosity divided by porosity on the face
  ADReal mu_eps_face;
  if (!_effective_viscosity)
    interpolate(Moose::FV::InterpMethod::Average,
                mu_eps_face,
                (*_mu_elem)[_qp] / _eps[_qp],
                (*_mu_neighbor)[_qp] / _eps_neighbor[_qp],
                *_face_info,
                true);
  else
    interpolate(Moose::FV::InterpMethod::Average,
                mu_eps_face,
                (*_mu_eff_elem)[_qp],
                (*_mu_eff_neighbor)[_qp],
                *_face_info,
                true);

  // Compute face superficial velocity gradient
  auto dudn = gradUDotNormal();

  // First term of residual
  ADReal residual = mu_eps_face * dudn;

  // Add the porosity gradient term if requested
  if (_smooth_porosity) {

    // Treat the porosity gradient separately
    const auto & grad_eps_face = MetaPhysicL::raw_value(_eps_var->adGradSln(*_face_info));

    ADRealVectorValue term_elem = (*_mu_elem)[_qp] / _eps[_qp] / _eps[_qp] *
        grad_eps_face(_index) * _vel_elem[_qp];
    ADRealVectorValue term_neighbor = (*_mu_neighbor)[_qp] / _eps_neighbor[_qp] / _eps_neighbor[_qp] *
        grad_eps_face(_index) * _vel_neighbor[_qp];

    // Interpolate to get the face value
    ADRealVectorValue term_face;
    interpolate(Moose::FV::InterpMethod::Average,
                term_face,
                term_elem,
                term_neighbor,
                *_face_info,
                true);
    residual -= term_face * _normal;
  }

  return -residual;
#endif
}
