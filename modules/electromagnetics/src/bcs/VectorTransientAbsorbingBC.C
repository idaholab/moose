//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorTransientAbsorbingBC.h"
#include "ElectromagneticConstants.h"
#include "ElectromagneticEnums.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", VectorTransientAbsorbingBC);

InputParameters
VectorTransientAbsorbingBC::validParams()
{
  InputParameters params = VectorIntegratedBC::validParams();
  params.addClassDescription(
      "First order transient absorbing boundary condition for vector variables.");
  params.addParam<FunctionName>("admittance",
                                1 / (EM::mu_0 * EM::c),
                                "Real component of the intrinsic admittance of the infinite medium "
                                "(default is $\\sqrt{\\frac{\\epsilon_0}{\\mu_0}} = "
                                "\\frac{1}{\\mu_0 c}$, or the admittance of free space).");
  params.deprecateParam("admittance", "admittance_real", "06/01/2027");
  params.addParam<FunctionName>(
      "admittance_imag",
      "0",
      "Imaginary component of the intrinsic admittance of the infinite medium.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>(
      "component", component, "Variable field component (real or imaginary).");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

VectorTransientAbsorbingBC::VectorTransientAbsorbingBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),

    _admittance_real(getFunction("admittance_real")),
    _admittance_imag(getFunction("admittance_imag")),

    _component(getParam<MooseEnum>("component")),

    _coupled_val(coupledVectorValue("coupled_field")),
    _coupled_var_num(coupled("coupled_field")),

    _u_dot(dot()),
    _coupled_dot(coupledVectorDot("coupled_field")),
    _du_dot_du(dotDu()),
    _coupled_dot_du(coupledVectorDotDu("coupled_field"))
{
}

Real
VectorTransientAbsorbingBC::computeQpResidual()
{
  // Initialize field_dot components
  std::complex<double> field_dot_0(0, 0);
  std::complex<double> field_dot_1(0, 0);
  std::complex<double> field_dot_2(0, 0);

  // Create E_dot for residual based on component parameter
  if (_component == EM::REAL)
  {
    field_dot_0.real(_u_dot[_qp](0));
    field_dot_0.imag(_coupled_dot[_qp](0));

    field_dot_1.real(_u_dot[_qp](1));
    field_dot_1.imag(_coupled_dot[_qp](1));

    field_dot_2.real(_u_dot[_qp](2));
    field_dot_2.imag(_coupled_dot[_qp](2));
  }
  else
  {
    field_dot_0.real(_coupled_dot[_qp](0));
    field_dot_0.imag(_u_dot[_qp](0));

    field_dot_1.real(_coupled_dot[_qp](1));
    field_dot_1.imag(_u_dot[_qp](1));

    field_dot_2.real(_coupled_dot[_qp](2));
    field_dot_2.imag(_u_dot[_qp](2));
  }
  VectorValue<std::complex<double>> field_dot(field_dot_0, field_dot_1, field_dot_2);

  // Complex admittance of the infinite medium
  const std::complex<double> admittance(_admittance_real.value(_t, _q_point[_qp]),
                                        _admittance_imag.value(_t, _q_point[_qp]));

  // Calculate solution field contribution to BC residual
  std::complex<double> p_dot_test =
      EM::mu_0 * admittance *
      (_test[_i][_qp].cross(_normals[_qp]) * _normals[_qp].cross(field_dot));

  std::complex<double> res = -p_dot_test;

  if (_component == EM::REAL)
    return res.real();
  else
    return res.imag();
}

Real
VectorTransientAbsorbingBC::computeQpJacobian()
{
  RealVectorValue prefix =
      EM::mu_0 * _admittance_real.value(_t, _q_point[_qp]) * _test[_i][_qp].cross(_normals[_qp]);

  return -prefix * _normals[_qp].cross(_du_dot_du[_qp] * _phi[_j][_qp]);
}

Real
VectorTransientAbsorbingBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _coupled_var_num)
  {
    // The coupled field enters through the imaginary part of the admittance, with a positive
    // sign in the real component of the residual and a negative sign in the imaginary component
    const Real sign = (_component == EM::REAL) ? 1.0 : -1.0;
    RealVectorValue prefix = sign * EM::mu_0 * _admittance_imag.value(_t, _q_point[_qp]) *
                             _test[_i][_qp].cross(_normals[_qp]);

    return prefix * _normals[_qp].cross(_coupled_dot_du[_qp] * _phi[_j][_qp]);
  }
  else
    return 0.0;
}
