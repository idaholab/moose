//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorTransientAbsorbingBC.h"
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
                                "1/(4*pi*1e-7*3e8)",
                                "Intrinsic admittance of the infinite medium (default is "
                                "$\\sqrt{\\frac{\\epsilon_0}{\\mu_0}} = \\frac{1}{\\mu_0 c}$, or "
                                "the admittance of free space).");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>(
      "component", component, "Variable field component (real or imaginary).");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

VectorTransientAbsorbingBC::VectorTransientAbsorbingBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),

    _admittance(getFunction("admittance")),
    _mu0(4.0 * libMesh::pi * 1.0e-7),

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

  // Calculate solution field contribution to BC residual
  std::complex<double> p_dot_test = _mu0 * _admittance.value(_t, _q_point[_qp]) *
                                    _test[_i][_qp].cross(_normals[_qp]) *
                                    _normals[_qp].cross(field_dot);

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
      _mu0 * _admittance.value(_t, _q_point[_qp]) * _test[_i][_qp].cross(_normals[_qp]);

  return prefix * _normals[_qp].cross(_du_dot_du[_qp] * _phi[_j][_qp]);
}

Real
VectorTransientAbsorbingBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _coupled_var_num)
  {
    RealVectorValue prefix =
        _mu0 * _admittance.value(_t, _q_point[_qp]) * _test[_i][_qp].cross(_normals[_qp]);

    return prefix * _normals[_qp].cross(_coupled_dot_du[_qp] * _phi[_j][_qp]);
  }
  else
    return 0.0;
}
