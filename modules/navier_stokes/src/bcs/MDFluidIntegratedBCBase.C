//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidIntegratedBCBase.h"
#include "MooseMesh.h"

InputParameters
MDFluidIntegratedBCBase::validParams()
{
  InputParameters params = IntegratedBC::validParams();

  // Coupled variables
  params.addRequiredCoupledVar("u", "velocity in x-coord");
  params.addCoupledVar("v", "velocity in y-coord"); // required in 2D and 3D
  params.addCoupledVar("w", "velocity in z-coord"); // required in 3D

  params.addRequiredCoupledVar("pressure", "pressure");
  params.addRequiredCoupledVar("temperature", "temperature");
  params.addCoupledVar("porosity", "porosity");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  return params;
}

MDFluidIntegratedBCBase::MDFluidIntegratedBCBase(const InputParameters & parameters)
  : IntegratedBC(parameters),
    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _rho(getMaterialProperty<Real>("rho_fluid")),
    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),
    _grad_pressure(coupledGradient("pressure")),
    _grad_temperature(coupledGradient("temperature")),
    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
    _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),
    _pressure_var_number(coupled("pressure")),
    _temperature_var_number(coupled("temperature")),

    _has_porosity(isParamValid("porosity")),
    _porosity(_has_porosity ? coupledValue("porosity") : _zero),

    _eos(getUserObject<SinglePhaseFluidProperties>("eos"))
{
}

unsigned
MDFluidIntegratedBCBase::map_var_number(unsigned var)
{
  // Convert the Moose numbering to:
  // 0 for pressure
  // 1 for u
  // 2 for v
  // 3 for w
  // 4 for temperature
  // regardless of the problem dimension, etc.
  unsigned mapped_var_number = 99;

  if (var == _pressure_var_number)
    mapped_var_number = 0;
  else if (var == _u_vel_var_number)
    mapped_var_number = 1;
  else if (var == _v_vel_var_number)
    mapped_var_number = 2;
  else if (var == _w_vel_var_number)
    mapped_var_number = 3;
  else if (var == _temperature_var_number)
    mapped_var_number = 4;
  else
    mapped_var_number = 99;

  return mapped_var_number;
}
