//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvectionSide.h"

registerMooseObject("MooseApp", ADHDGAdvectionSide);

InputParameters
ADHDGAdvectionSide::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addParam<Real>(
      "coeff", 1, "A constant coefficient. This could be something like a density");
  params.addRequiredCoupledVar("interior_variable", "interior variable to find jumps in");
  params.addParam<bool>("self_advection",
                        true,
                        "Whether this kernel should advect itself, e.g. it's "
                        "variable/interior_variable pair. If false, we will advect "
                        "unity (possibly multiplied by the 'coeff' parameter");
  return params;
}

ADHDGAdvectionSide::ADHDGAdvectionSide(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _velocity_neighbor(getNeighborADMaterialProperty<RealVectorValue>("velocity")),
    _coeff(getParam<Real>("coeff")),
    _interior_value(isParamValid("interior_variable") ? &adCoupledValue("interior_variable")
                                                      : nullptr),
    _interior_neighbor_value(
        isParamValid("interior_variable") ? &adCoupledNeighborValue("interior_variable") : nullptr)
{
  if (_interior_value && !getParam<bool>("self_advection"))
    paramError("interior_variable",
               "If not advecting the variable/interior_variable pair as indicated by "
               "'self_advection=false', then 'interior_variable' should not supplied");
  else if (!_interior_value && getParam<bool>("self_advection"))
    paramError(
        "interior_variable",
        "When advecting a variable/interior_variable pair as indicated by 'self_advection=true' "
        "(the default), then 'interior_variable' should be supplied");
}

ADReal
ADHDGAdvectionSide::computeQpResidual(Moose::DGResidualType type)
{
  ADReal element_r = 0;
  ADReal neighbor_r = 0;

  switch (type)
  {
    case Moose::Element:
    {
      {
        const auto vdotn = _velocity[_qp] * _normals[_qp];
        if (_interior_value)
        {
          if (MetaPhysicL::raw_value(vdotn) >= 0)
            // outflow
            element_r = (*_interior_value)[_qp];
          else
            // inflow
            element_r = _u[_qp];
        }
        else
          element_r = 1;

        element_r *= -_coeff * _test[_i][_qp] * vdotn;
      }
      {
        const auto vdotn = _velocity_neighbor[_qp] * -_normals[_qp];
        if (_interior_neighbor_value)
        {
          if (MetaPhysicL::raw_value(vdotn) >= 0)
            // outflow
            neighbor_r = (*_interior_neighbor_value)[_qp];
          else
            // inflow
            neighbor_r = _u[_qp];
        }
        else
          neighbor_r = 1;

        neighbor_r *= -_coeff * _test[_i][_qp] * vdotn;
      }
      break;
    }

    case Moose::Neighbor:
      break;
  }

  return element_r + neighbor_r;
}
