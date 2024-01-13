//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionFluxAux.h"
#include "Assembly.h"

registerMooseObject("MooseApp", DiffusionFluxAux);

InputParameters
DiffusionFluxAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum component("x y z normal");
  params.addClassDescription("Compute components of flux vector for diffusion problems "
                             "$(\\vec{J} = -D \\nabla C)$.");
  params.addRequiredParam<MooseEnum>("component", component, "The desired component of flux.");
  params.addRequiredCoupledVar("diffusion_variable", "The name of the variable");
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  return params;
}

DiffusionFluxAux::DiffusionFluxAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _use_normal(getParam<MooseEnum>("component") == "normal"),
    _component(getParam<MooseEnum>("component")),
    _grad_u(coupledGradient("diffusion_variable")),
    _diffusion_coef(hasMaterialProperty<Real>("diffusivity")
                        ? &getMaterialProperty<Real>("diffusivity")
                        : nullptr),
    _ad_diffusion_coef(!_diffusion_coef ? &getADMaterialProperty<Real>("diffusivity") : nullptr),
    _normals(_assembly.normals())
{
  if (_use_normal && !isParamValid("boundary"))
    paramError("boundary", "A boundary must be provided if using the normal component!");
}

Real
DiffusionFluxAux::computeValue()
{
  const Real gradient = _use_normal ? _grad_u[_qp] * _normals[_qp] : _grad_u[_qp](_component);
  const Real diffusion_coef = _diffusion_coef ? (*_diffusion_coef)[_qp]
                                              : MetaPhysicL::raw_value((*_ad_diffusion_coef)[_qp]);
  return -diffusion_coef * gradient;
}
