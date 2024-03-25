//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvection.h"

registerMooseObject("MooseApp", ADHDGAdvection);

InputParameters
ADHDGAdvection::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addParam<Real>(
      "coeff", 1, "A constant coefficient. This could be something like a density");
  params.addCoupledVar("side_variable", "side variable to use as Lagrange multiplier");
  params.addParam<bool>("self_advection",
                        true,
                        "Whether this kernel should advect itself, e.g. it's "
                        "variable/side_variable pair. If false, we will advect "
                        "unity (possibly multiplied by the 'coeff' parameter");
  return params;
}

ADHDGAdvection::ADHDGAdvection(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _velocity_neighbor(getNeighborADMaterialProperty<RealVectorValue>("velocity")),
    _coeff(getParam<Real>("coeff")),
    _side_u(isParamValid("side_variable") ? &adCoupledValue("side_variable") : nullptr)
{
  if (_side_u && !getParam<bool>("self_advection"))
    paramError("side_variable",
               "If not advecting the variable/side_variable pair as indicated by "
               "'self_advection=false', then 'side_variable' should not supplied");
  else if (!_side_u && getParam<bool>("self_advection"))
    paramError("side_variable",
               "When advecting a variable/side_variable pair as indicated by 'self_advection=true' "
               "(the default), then 'side_variable' should be supplied");
}

ADReal
ADHDGAdvection::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0.0;

  switch (type)
  {
    case Moose::Element:
    {
      const auto vdotn = _velocity[_qp] * _normals[_qp];
      if (_side_u)
      {
        if (MetaPhysicL::raw_value(vdotn) >= 0)
          // outflow
          r = _u[_qp];
        else
          // inflow
          r = (*_side_u)[_qp];
      }
      else
        r = 1;

      r *= _coeff * _test[_i][_qp] * vdotn;
      break;
    }

    case Moose::Neighbor:
    {
      const auto vdotn = _velocity_neighbor[_qp] * -_normals[_qp];
      if (_side_u)
      {
        if (MetaPhysicL::raw_value(vdotn) >= 0)
          // outflow
          r = _u_neighbor[_qp];
        else
          // inflow
          r = (*_side_u)[_qp];
      }
      else
        r = 1;

      r *= _coeff * _test_neighbor[_i][_qp] * vdotn;
      break;
    }
  }

  return r;
}
