//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidKernelBase.h"
#include "MooseMesh.h"
#include "SystemBase.h"

InputParameters
MDFluidKernelBase::validParams()
{
  InputParameters params = Kernel::validParams();

  // Coupled variables
  params.addRequiredCoupledVar("u", "velocity in x-direction");
  params.addCoupledVar("v", "velocity in y-direction"); // required in 2D/3D
  params.addCoupledVar("w", "velocity in z-direction"); // required in 3D

  params.addRequiredCoupledVar("pressure", "pressure");
  params.addRequiredCoupledVar("temperature", "temperature");
  params.addCoupledVar("porosity", "porosity");

  params.addParam<bool>("transient", true, "if it is a transient simulation.");
  params.addParam<VectorValue<Real>>("gravity", "Gravity vector");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  return params;
}

MDFluidKernelBase::MDFluidKernelBase(const InputParameters & parameters)
  : Kernel(parameters),
    _second_u(_var.secondSln()),
    // Coupled variables
    _u_var(_sys.getFieldVariable<Real>(_tid, parameters.get<std::vector<VariableName>>("u")[0])),
    _v_var(_sys.getFieldVariable<Real>(_tid, parameters.get<std::vector<VariableName>>("v")[0])),
    _w_var(_mesh.dimension() == 3 ? _sys.getFieldVariable<Real>(
                                        _tid, parameters.get<std::vector<VariableName>>("w")[0])
                                  : _u_var),

    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _rho(getMaterialProperty<Real>("rho_fluid")),

    _has_porosity(isParamValid("porosity")),
    _porosity(_has_porosity ? coupledValue("porosity") : _zero),

    _bTransient(getParam<bool>("transient")),
    _u_vel_dot(_bTransient ? coupledDot("u") : _zero),
    _v_vel_dot(_bTransient && (_mesh.dimension() >= 2) ? coupledDot("v") : _zero),
    _w_vel_dot(_bTransient && (_mesh.dimension() == 3) ? coupledDot("w") : _zero),

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

    // Material properties
    _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
    _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
    _turbulence_viscosity(getMaterialProperty<Real>("turbulence_viscosity")),
    _inertia_resistance_coeff(getMaterialProperty<RealTensorValue>("inertia_resistance_coeff")),
    _viscous_resistance_coeff(getMaterialProperty<RealTensorValue>("viscous_resistance_coeff")),
    _eos(getUserObject<SinglePhaseFluidProperties>("eos")),
    _vec_g(0., 0., 0.)
{
  if (isParamValid("gravity"))
    _vec_g = getParam<VectorValue<Real>>("gravity");
  else if (_mesh.dimension() == 2)
    _vec_g(1) = -9.8;
  else if (_mesh.dimension() == 3)
    _vec_g(2) = -9.8;
}

unsigned
MDFluidKernelBase::map_var_number(unsigned var)
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
