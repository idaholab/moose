//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NS.h"
#include "INSPressureGradientDGKernel.h"

registerMooseObject("NavierStokesApp", INSPressureGradientDGKernel);

InputParameters
INSPressureGradientDGKernel::validParams()
{
  InputParameters params = ADDGKernel::validParams();

  params.addClassDescription("Adds the pressure term on a interior faces. This is appropriate when "
                             "the pressure term is integrated by parts");
  params.addRequiredCoupledVar(NS::pressure, "The current value of the pressure");
  params.addRequiredParam<unsigned>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");

  return params;
}

INSPressureGradientDGKernel::INSPressureGradientDGKernel(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _pressure(adCoupledValue(NS::pressure)),
    _pressure_neighbor(adCoupledNeighborValue(NS::pressure)),
    _component(getParam<unsigned>("component"))
{
}

ADReal
INSPressureGradientDGKernel::computeQpResidual(const Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
    case Moose::Neighbor:
      return -_pressure_neighbor[_qp] * _normals[_qp](_component) * _test_neighbor[_i][_qp];
    default:
      libmesh_assert(false);
      return 0;
  }
}
