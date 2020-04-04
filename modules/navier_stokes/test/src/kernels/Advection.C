//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Advection.h"
#include "Function.h"

registerMooseObject("NavierStokesTestApp", Advection);

InputParameters
Advection::validParams()
{
  InputParameters params = INSBase::validParams();

  params.addClassDescription("This class solves the scalar advection equation, "
                             "$\\vec{a}\\cdot\\nabla u = f$ with SUPG stabilization.");
  params.addParam<FunctionName>("forcing_func", 0, "The forcing function, typically used for MMS.");
  MooseEnum tau_type("opt mod");
  params.addRequiredParam<MooseEnum>(
      "tau_type", tau_type, "The type of stabilization parameter to use.");
  return params;
}

Advection::Advection(const InputParameters & parameters)
  : INSBase(parameters),
    _ffn(getFunction("forcing_func")),
    _tau_type(getParam<MooseEnum>("tau_type"))
{
}

Real
Advection::computeQpResidual()
{
  Real tau_val = (_tau_type == "opt" ? tauNodal() : tau());
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return (_test[_i][_qp] + tau_val * (U * _grad_test[_i][_qp])) *
         (U * _grad_u[_qp] - _ffn.value(_t, _q_point[_qp]));
}

Real
Advection::computeQpJacobian()
{
  Real tau_val = (_tau_type == "opt" ? tauNodal() : tau());
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return (_test[_i][_qp] + tau_val * (U * _grad_test[_i][_qp])) * (U * _grad_phi[_j][_qp]);
}
