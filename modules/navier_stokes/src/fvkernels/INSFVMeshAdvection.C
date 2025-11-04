//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMeshAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMeshAdvection);

InputParameters
INSFVMeshAdvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Implements a source/sink term for this object's variable/advected-quantity "
      "proportional to the divergence of the mesh velocity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density. This should be constant");
  params.addRequiredParam<MooseFunctorName>("disp_x", "The x-displacement");
  params.addParam<MooseFunctorName>("disp_y", 0, "The y-displacement");
  params.addParam<MooseFunctorName>("disp_z", 0, "The z-displacement");
  params.addParam<MooseFunctorName>(
      "advected_quantity",
      "An optional parameter for a functor describing the advected quantity. If this is not "
      "provided, then the 'variable' will be used");
  return params;
}

INSFVMeshAdvection::INSFVMeshAdvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _rho(getFunctor<ADReal>(NS::density)),
    _disp_x(getFunctor<ADReal>("disp_x")),
    _disp_y(getFunctor<ADReal>("disp_y")),
    _disp_z(getFunctor<ADReal>("disp_z")),
    _adv_quant(isParamValid("advected_quantity") ? static_cast<const Moose::FunctorBase<ADReal> &>(
                                                       getFunctor<ADReal>("advected_quantity"))
                                                 : static_cast<Moose::FunctorBase<ADReal> &>(_var))
{
}

ADReal
INSFVMeshAdvection::advQuantCoeff(const Moose::ElemArg & elem_arg,
                                  const Moose::StateArg & state) const
{
  const auto div_mesh_velocity = _disp_x.gradDot(elem_arg, state)(0) +
                                 _disp_y.gradDot(elem_arg, state)(1) +
                                 _disp_z.gradDot(elem_arg, state)(2);
  return _rho(elem_arg, state) * div_mesh_velocity;
}

ADReal
INSFVMeshAdvection::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return advQuantCoeff(elem_arg, state) * _adv_quant(elem_arg, state);
}
