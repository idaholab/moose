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
    _mu_elem(getADMaterialProperty<Real>("mu")),
    _mu_neighbor(getNeighborADMaterialProperty<Real>("mu")),
    _eps(coupledValue("porosity")),
    _eps_neighbor(coupledNeighborValue("porosity")),
    _index(getParam<MooseEnum>("momentum_component")),
    _vel_elem(isParamValid("vel") ? &getADMaterialProperty<RealVectorValue>("vel") : nullptr),
    _vel_neighbor(isParamValid("vel") ? &getNeighborADMaterialProperty<RealVectorValue>("vel")
                                      : nullptr),
    _eps_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar("porosity", 0))),
    _smooth_porosity(getParam<bool>("smooth_porosity"))
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
  // Interpolate viscosity divided by porosity on the face
  ADReal mu_eps_face;
  interpolate(Moose::FV::InterpMethod::Average,
              mu_eps_face,
              _mu_elem[_qp] / _eps[_qp],
              _mu_neighbor[_qp] / _eps_neighbor[_qp],
              *_face_info,
              true);

  // Compute face superficial velocity gradient
  auto dudn = gradUDotNormal();

  // First term of residual
  ADReal residual = mu_eps_face * dudn;

  if (_smooth_porosity)
  {
    // Get the face porosity gradient separately
    const auto & grad_eps_face = MetaPhysicL::raw_value(_eps_var->adGradSln(*_face_info));

    ADRealVectorValue term_elem = _mu_elem[_qp] / _eps[_qp] / _eps[_qp] * grad_eps_face;
    ADRealVectorValue term_neighbor =
        _mu_neighbor[_qp] / _eps_neighbor[_qp] / _eps_neighbor[_qp] * grad_eps_face;
    for (int i = 0; i < LIBMESH_DIM; i++)
    {
      term_elem(i) *= (*_vel_elem)[_qp](i);
      term_neighbor(i) *= (*_vel_neighbor)[_qp](i);
    }

    // Interpolate to get the face value
    ADRealVectorValue term_face;
    interpolate(
        Moose::FV::InterpMethod::Average, term_face, term_elem, term_neighbor, *_face_info, true);
    residual -= term_face * _normal;
  }
  return -residual;
#else
  return 0;
#endif
}
