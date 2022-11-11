//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidEnergyDirichletBC.h"

registerMooseObject("NavierStokesApp", MDFluidEnergyDirichletBC);

InputParameters
MDFluidEnergyDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();

  params.addRequiredCoupledVar("u", "velocity in x-direction");
  params.addCoupledVar("v", "velocity in y-direction"); // required in 2D/3D
  params.addCoupledVar("w", "velocity in z-direction"); // required in 3D

  params.addParam<FunctionName>("v_fn", "Velocity function with time at the boundary");
  params.addParam<FunctionName>("T_fn", "Temperature function with time at the boundary");

  params.addRequiredParam<VectorValue<Real>>("out_norm", "out norm of the boundary");

  return params;
}

MDFluidEnergyDirichletBC::MDFluidEnergyDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _out_norm(getParam<VectorValue<Real>>("out_norm")),
    _u_vel(coupledValueOld("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValueOld("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValueOld("w") : _zero),
    _has_vbc(parameters.isParamValid("v_fn")),
    _has_Tbc(parameters.isParamValid("T_fn")),
    _velocity_fn(_has_vbc ? &getFunction("v_fn") : NULL),
    _temperature_fn(_has_Tbc ? &getFunction("T_fn") : NULL)
{
}

bool
MDFluidEnergyDirichletBC::isInlet()
{
  RealVectorValue vec_vel(_u_vel[0], _v_vel[0], _w_vel[0]);

  Real v_bc = 0.0;
  if (_has_vbc)
    v_bc = -_velocity_fn->value(_t, *_current_node);
  else
    v_bc = vec_vel * _out_norm;

  if (v_bc < 0.) // Inlet
    return true;
  else
    return false;
}

bool
MDFluidEnergyDirichletBC::shouldApply()
{
  return isInlet();
}

Real
MDFluidEnergyDirichletBC::computeQpResidual()
{
  if (isInlet())
    return _u[_qp] - _temperature_fn->value(_t, *_current_node);
  else
    return 0.0;
}

Real
MDFluidEnergyDirichletBC::computeQpJacobian()
{
  return isInlet() ? 1.0 : 0.0;
}

Real
MDFluidEnergyDirichletBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}
