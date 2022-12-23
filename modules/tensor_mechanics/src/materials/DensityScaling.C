//* This file is part of the MOOSE framework
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DensityScaling.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", DensityScaling);

InputParameters
DensityScaling::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "density",
      "Name of Material Property or a constant real number defining the density of the material.");
  params.addRequiredParam<Real>("desired_time_step", "Time step to achieve.");
  params.addParam<Real>(
      "factor",
      1.0,
      "Factor to multiply to the critical time step. This factor is typically less than one to be "
      "on the conservative side due to two types of approximation: Time step lagging and the "
      "approximation of the critical time step formula.");
  return params;
}

DensityScaling::DensityScaling(const InputParameters & parameters)
  : Material(parameters),
    _desired_time_step(getParam<Real>("desired_time_step")),
    _density_scaling(declareProperty<Real>("density_scaling")),
    _material_density(getMaterialPropertyByName<Real>("density")),
    _effective_stiffness(getMaterialPropertyByName<Real>("effective_stiffness")),
    _factor(getParam<Real>("factor"))
{
  mooseInfo("Since it can change key simulation results, usage of selective density (mass) scaling "
            "is only recommended for advanced users.");
}

void
DensityScaling::computeQpProperties()
{
  const Real critical = _factor * _current_elem->hmin() * std::sqrt(_material_density[_qp]) /
                        (_effective_stiffness[0]);

  if (critical < _desired_time_step)
  {
    const Real desired_density = std::pow(_effective_stiffness[_qp] * _desired_time_step, 2) /
                                 std::pow(_factor * _current_elem->hmin(), 2);

    const Real density_to_add =
        desired_density > _material_density[_qp] ? desired_density - _material_density[_qp] : 0.0;

    _density_scaling[_qp] = density_to_add;
  }
  else
    _density_scaling[_qp] = 0.0;
}
