//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectiveFluxAux.h"
#include "Assembly.h"

registerMooseObject("MooseApp", AdvectiveFluxAux);

InputParameters
AdvectiveFluxAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum component("x y z normal");

  params.addRequiredParam<MooseEnum>("component", component, "The desired component of flux.");
  params.addCoupledVar("advected_variable", 0, "The name of the variable");
  params.addRequiredParam<MooseFunctorName>("vel_x", "x-component of the vector");
  params.addParam<MooseFunctorName>("vel_y", "y-component of the vector");
  params.addParam<MooseFunctorName>("vel_z", "z-component of the vector");
  params.addParam<MooseFunctorName>("advected_mat_prop",
                                    0,
                                    "The advected material property of which to study the flow; "
                                    "useful for finite element simulations");

  params.addClassDescription("Compute components of flux vector for advection problems "
                             "$(\\vec{J} = \\vec{v} u \\cdot \\vec{n})$.");

  return params;
}

AdvectiveFluxAux::AdvectiveFluxAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _use_normal(getParam<MooseEnum>("component") == "normal"),
    _component(getParam<MooseEnum>("component")),
    _advected_variable(coupledValue("advected_variable")),
    _normals(_assembly.normals()),
    _vel_x(getFunctor<ADReal>("vel_x")),
    _vel_y(_mesh.dimension() >= 2 ? &getFunctor<ADReal>("vel_y") : nullptr),
    _vel_z(_mesh.dimension() == 3 ? &getFunctor<ADReal>("vel_z") : nullptr),
    _advected_variable_supplied(parameters.isParamSetByUser("advected_variable")),
    _advected_mat_prop_supplied(parameters.isParamSetByUser("advected_mat_prop")),
    _advected_material_property(getFunctor<ADReal>("advected_mat_prop"))

{
  if (_use_normal && !isParamValid("boundary"))
    paramError("boundary", "A boundary must be provided if using the normal component!");
  if (_advected_variable_supplied && _advected_mat_prop_supplied)
    mooseError("AdvectiveFluxAux should be provided either an advected variable "
               "or an advected material property");
  if (dynamic_cast<MooseVariableFV<Real> *>(&_var))
    mooseError("AdvectiveFluxAux is designed to use for finite element simulations.");
}

Real
AdvectiveFluxAux::computeValue()
{
  using MetaPhysicL::raw_value;

  const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
  const auto state = determineState();

  auto vel_x = raw_value(_vel_x(qp_arg, state));
  auto vel_y = _vel_y ? raw_value((*_vel_y)(qp_arg, state)) : 0;
  auto vel_z = _vel_z ? raw_value((*_vel_z)(qp_arg, state)) : 0;

  if (_advected_variable_supplied)
    return (_use_normal
                ? _advected_variable[_qp] * RealVectorValue(vel_x, vel_y, vel_z) * _normals[_qp]
                : _advected_variable[_component] *
                      RealVectorValue(vel_x, vel_y, vel_z)(_component));
  else if (_advected_mat_prop_supplied)
    return (_use_normal ? raw_value(_advected_material_property(qp_arg, state)) *
                              RealVectorValue(vel_x, vel_y, vel_z) * _normals[_qp]
                        : raw_value(_advected_material_property(qp_arg, state)) *
                              RealVectorValue(vel_x, vel_y, vel_z)(_component));
  else
    return (_use_normal ? RealVectorValue(vel_x, vel_y, vel_z) * _normals[_qp]
                        : RealVectorValue(vel_x, vel_y, vel_z)(_component));
}
