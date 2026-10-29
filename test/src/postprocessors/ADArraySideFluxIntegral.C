//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArraySideFluxIntegral.h"

registerMooseObject("MooseTestApp", ADArraySideFluxIntegral);

InputParameters
ADArraySideFluxIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("v", "The coupled array variable");
  params.addRequiredParam<unsigned int>("component", "The array component to integrate");
  params.addClassDescription("Integrates $\\nabla v_i \\cdot \\hat{n}$ of a component of a "
                             "coupled array variable over a boundary using the automatic "
                             "differentiation gradient.");
  return params;
}

ADArraySideFluxIntegral::ADArraySideFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _grad_v(adCoupledArrayGradient("v")),
    _component(getParam<unsigned int>("component"))
{
  if (_component >= getArrayVar("v", 0)->count())
    paramError("component", "Exceeds the number of components of 'v'");
}

Real
ADArraySideFluxIntegral::computeQpIntegral()
{
  Real flux = 0;
  for (const auto d : make_range(Moose::dim))
    flux += MetaPhysicL::raw_value(_grad_v[_qp](_component, d)) * _normals[_qp](d);
  return flux;
}
