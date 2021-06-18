//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorEMRobinBC.h"
#include "ElectromagneticEnums.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", VectorEMRobinBC);

InputParameters
VectorEMRobinBC::validParams()
{
  InputParameters params = VectorIntegratedBC::validParams();
  params.addClassDescription(
      "First order Robin-style Absorbing/Port BC for vector variables.");
  params.addParam<FunctionName>(
      "beta", 1.0, "Scalar waveguide propagation constant (usually some k, k0).");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>(
      "component", component, "Variable field component (real or imaginary).");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  params.addParam<FunctionName>("real_incoming", 0.0, "Real incoming field vector.");
  params.addParam<FunctionName>("imag_incoming", 0.0, "Imaginary incoming field vector.");
  MooseEnum mode("absorbing port", "port");
  params.addParam<MooseEnum>(
      "mode",
      mode,
      "Mode of operation for VectorEMRobinBC. Can be set to 'absorbing' or 'port' "
      "(default: 'port').");
  return params;
}

VectorEMRobinBC::VectorEMRobinBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),

    _beta(getFunction("beta")),

    _component(getParam<MooseEnum>("component")),

    _coupled_val(coupledVectorValue("coupled_field")),
    _coupled_var_num(coupled("coupled_field")),

    _inc_real(getFunction("real_incoming")),
    _inc_imag(getFunction("imag_incoming")),

    // Value of complex j (can't use _j or _i due to MOOSE basis fxn conventions)
    _jay(0, 1),

    _mode(getParam<MooseEnum>("mode"))
{
  bool real_incoming_was_set = parameters.isParamSetByUser("real_incoming");
  bool imag_incoming_was_set = parameters.isParamSetByUser("imag_incoming");

  if (_mode == electromagnetics::ABSORBING && (real_incoming_was_set || imag_incoming_was_set))
  {
    mooseError(
        "In ",
        _name,
        ", mode was set to Absorbing, while an incoming profile function (used for Port BCs) was "
        "defined. Either remove the profile function parameters, or set your BC to Port mode!");
  }
}

Real
VectorEMRobinBC::computeQpResidual()
{

  // Initialize E components
  std::complex<double> field_0(0, 0);
  std::complex<double> field_1(0, 0);
  std::complex<double> field_2(0, 0);

  // Create E and ncrossE for residual based on component parameter
  if (_component == electromagnetics::REAL)
  {
    field_0.real(_u[_qp](0));
    field_0.imag(_coupled_val[_qp](0));

    field_1.real(_u[_qp](1));
    field_1.imag(_coupled_val[_qp](1));

    field_2.real(_u[_qp](2));
    field_2.imag(_coupled_val[_qp](2));
  }
  else
  {
    field_0.real(_coupled_val[_qp](0));
    field_0.imag(_u[_qp](0));

    field_1.real(_coupled_val[_qp](1));
    field_1.imag(_u[_qp](1));

    field_2.real(_coupled_val[_qp](2));
    field_2.imag(_u[_qp](2));
  }
  VectorValue<std::complex<double>> field(field_0, field_1, field_2);

  // Creating vector, curl for field_inc before residual and Jacobian contributions
  std::complex<double> field_inc_0(_inc_real.vectorValue(_t, _q_point[_qp])(0),
                                   _inc_imag.vectorValue(_t, _q_point[_qp])(0));
  std::complex<double> field_inc_1(_inc_real.vectorValue(_t, _q_point[_qp])(1),
                                   _inc_imag.vectorValue(_t, _q_point[_qp])(1));
  std::complex<double> field_inc_2(_inc_real.vectorValue(_t, _q_point[_qp])(2),
                                   _inc_imag.vectorValue(_t, _q_point[_qp])(2));
  VectorValue<std::complex<double>> field_inc(field_inc_0, field_inc_1, field_inc_2);

  std::complex<double> curl_inc0(_inc_real.vectorCurl(_t, _q_point[_qp])(0),
                                 _inc_imag.vectorCurl(_t, _q_point[_qp])(0));
  std::complex<double> curl_inc1(_inc_real.vectorCurl(_t, _q_point[_qp])(1),
                                 _inc_imag.vectorCurl(_t, _q_point[_qp])(1));
  std::complex<double> curl_inc2(_inc_real.vectorCurl(_t, _q_point[_qp])(2),
                                 _inc_imag.vectorCurl(_t, _q_point[_qp])(2));
  VectorValue<std::complex<double>> curl_inc(curl_inc0, curl_inc1, curl_inc2);

  std::complex<double> u_inc_dot_test = 0.0;
  switch (_mode)
  {
    case electromagnetics::PORT:
      // Calculate incoming wave contribution to BC residual
      u_inc_dot_test = _test[_i][_qp].cross(_normals[_qp]) * curl_inc +
                       _jay * _beta.value(_t, _q_point[_qp]) *
                           (_test[_i][_qp].cross(_normals[_qp]) * _normals[_qp].cross(field_inc));
      break;
    case electromagnetics::ABSORBING:
      break;
  }

  // Calculate solution field contribution to BC residual (first order version)
  std::complex<double> p_dot_test = _jay * _beta.value(_t, _q_point[_qp]) *
                                    _test[_i][_qp].cross(_normals[_qp]) *
                                    _normals[_qp].cross(field);

  std::complex<double> diff = u_inc_dot_test - p_dot_test;
  Real res = 0.0;
  switch (_component)
  {
    case electromagnetics::REAL:
      res = diff.real();
      break;
    case electromagnetics::IMAGINARY:
      res = diff.imag();
      break;
  }
  return res;
}

Real
VectorEMRobinBC::computeQpJacobian()
{
  return 0.0;
}

Real
VectorEMRobinBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real off_diag_jac = _beta.value(_t, _q_point[_qp]) * _test[_i][_qp].cross(_normals[_qp]) *
                      _normals[_qp].cross(_phi[_j][_qp]);

  if (_component == electromagnetics::REAL && jvar == _coupled_var_num)
  {
    return off_diag_jac;
  }
  else if (_component == electromagnetics::IMAGINARY && jvar == _coupled_var_num)
  {
    return -off_diag_jac;
  }
  else
  {
    return 0.0;
  }
}
