//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegralDirectedSurfaceForceFE.h"
#include "NS.h"

#include <cmath>

registerMooseObject("NavierStokesApp", IntegralDirectedSurfaceForceFE);

InputParameters
IntegralDirectedSurfaceForceFE::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the directed force coming from friction and pressure differences on a surface. One "
      "can use this object for the computation of the drag and lift coefficient as well.");
  params.addRequiredCoupledVar("velocity", "The velocity.");
  params.addRequiredParam<MaterialPropertyName>(NS::mu, "The dynamic viscosity.");
  params.addRequiredCoupledVar(NS::pressure, "The pressure.");
  params.addRequiredParam<RealVectorValue>("principal_direction",
                                           "The direction in which the force is computed.");
  return params;
}

IntegralDirectedSurfaceForceFE::IntegralDirectedSurfaceForceFE(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _mu(getADMaterialProperty<Real>(NS::mu)),
    _pressure(coupledValue(NS::pressure)),
    _grad_vel(coupledVectorGradient("velocity")),
    _direction(getParam<RealVectorValue>("principal_direction"))
{
}

Real
IntegralDirectedSurfaceForceFE::computeQpIntegral()
{
  return _grad_vel[_qp] * _normals[_qp] * _direction * -MetaPhysicL::raw_value(_mu[_qp]) +
         _pressure[_qp] * _normals[_qp] * _direction;
}
