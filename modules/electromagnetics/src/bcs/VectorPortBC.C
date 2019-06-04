#include "VectorPortBC.h"
#include "ElkEnums.h"
#include <complex>

registerMooseObject("ElkApp", VectorPortBC);

template <>
InputParameters
validParams<VectorPortBC>()
{
  InputParameters params = validParams<VectorIntegratedBC>();
  params.addClassDescription("First order Absorbing/Port BC from 'Theory and Computation of "
                             "Electromagnetic Fields' by JM Jin.");
  params.addParam<FunctionName>(
      "beta", 1.0, "Scalar waveguide propagation constant (usually some k, k0).");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>(
      "component", component, "Variable field component (real or imaginary).");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  params.addParam<FunctionName>("real_incoming", 0.0, "Real incoming field vector.");
  params.addParam<FunctionName>("imag_incoming", 0.0, "Imaginary incoming field vector.");
  return params;
}

VectorPortBC::VectorPortBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),

    _beta(getFunction("beta")),

    _component(getParam<MooseEnum>("component")),

    _coupled_val(coupledVectorValue("coupled_field")),
    _coupled_var_num(coupled("coupled_field")),

    _inc_real(getFunction("real_incoming")),
    _inc_imag(getFunction("imag_incoming")),

    // Value of complex j (can't use _j or _i due to MOOSE basis fxn conventions)
    _jay(0, 1)
{
}

Real
VectorPortBC::computeQpResidual()
{

  // Initialize E components
  std::complex<double> field_0(0, 0);
  std::complex<double> field_1(0, 0);
  std::complex<double> field_2(0, 0);

  // Create E and ncrossE for residual based on component parameter
  if (_component == elk::REAL)
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

  // Calculate incoming wave contribution to BC residual
  std::complex<double> u_inc_dot_test =
      _test[_i][_qp].cross(_normals[_qp]) * curl_inc +
      _jay * _beta.value(_t, _q_point[_qp]) *
          (_test[_i][_qp].cross(_normals[_qp]) * _normals[_qp].cross(field_inc));

  // Calculate solution field contribution to BC residual (first order version)
  std::complex<double> p_dot_test = _jay * _beta.value(_t, _q_point[_qp]) *
                                    _test[_i][_qp].cross(_normals[_qp]) * _normals[_qp].cross(field);

  std::complex<double> res = u_inc_dot_test - p_dot_test;

  if (_component == elk::REAL)
  {
    return res.real();
  }
  else
  {
    return res.imag();
  }
}

Real
VectorPortBC::computeQpJacobian()
{
  return 0.0;
}

Real
VectorPortBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real off_diag_jac = _beta.value(_t, _q_point[_qp]) * _test[_i][_qp].cross(_normals[_qp]) *
                      _normals[_qp].cross(_phi[_j][_qp]);

  if (_component == elk::REAL && jvar == _coupled_var_num)
  {
    return off_diag_jac;
  }
  else if (_component == elk::IMAGINARY && jvar == _coupled_var_num)
  {
    return -off_diag_jac;
  }
  else
  {
    return 0.0;
  }
}
