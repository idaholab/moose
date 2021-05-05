//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumAdvectionPorosityGradient.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumAdvectionPorosityGradient);

InputParameters
PINSFVMomentumAdvectionPorosityGradient::validParams()
{
  auto params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Porosity gradient spun from the advection term for the porous media Navier Stokes "
      "momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");

  params.addRequiredCoupledVar("u", "The superficial velocity in the x direction.");
  params.addCoupledVar("v", "The superficial velocity in the y direction.");
  params.addCoupledVar("w", "The superficial velocity in the z direction.");

  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");

  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addRequiredParam<bool>("smooth_porosity", "Whether the porosity has no discontinuities");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumAdvectionPorosityGradient::PINSFVMomentumAdvectionPorosityGradient(
    const InputParameters & params)
  : FVElementalKernel(params),
    _eps_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar("porosity", 0))),
    _u(adCoupledValue("u")),
    _v(params.isParamValid("v") ? adCoupledValue("v") : _ad_zero),
    _w(params.isParamValid("w") ? adCoupledValue("w") : _ad_zero),
    _rho(getParam<Real>("rho")),
    _index(getParam<MooseEnum>("momentum_component")),
    _smooth_porosity(getParam<bool>("smooth_porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumAdvectionPorosityGradient may only be used with a superficial "
               "velocity variable, of variable type PINSFVSuperficialVelocityVariable.");

  if (!_smooth_porosity)
    paramError(
        "smooth_porosity",
        "The MomentumAdvectionContinuousPorosity may only be used with a continuous porosity.");
}

ADReal
PINSFVMomentumAdvectionPorosityGradient::computeQpResidual()
{
  const Real one_over_eps = 1 / MetaPhysicL::raw_value(_eps_var->getElemValue(_current_elem));
  ADRealVectorValue V = {_u[_qp], _v[_qp], _w[_qp]};

  return _rho * V(_index) * (-one_over_eps * one_over_eps) *
         (V * MetaPhysicL::raw_value(_eps_var->adGradSln(_current_elem)));
}
