#include "VectorPortBC.h"

registerMooseObject("ElkApp", VectorPortBC);

template <>
InputParameters
validParams<VectorPortBC>()
{
  InputParameters params = validParams<VectorIntegratedBC>();
  params.addClassDescription(
      "First order Port BC from Jin 'Theory and Computation of Electromagnetic Fields' by JM Jin.");
  params.addRequiredParam<FunctionName>("beta", "Waveguide propagation constant.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>(
      "component", component, "Variable field component (real or imaginary).");
  params.addRequiredCoupledVar("coupled", "Coupled field variable.");
  params.addParam<FunctionName>("Inc_real_x", 0.0, "Real incoming field, x direction.");
  params.addParam<FunctionName>("Inc_real_y", 0.0, "Real incoming field, y direction.");
  params.addParam<FunctionName>("Inc_real_z", 0.0, "Real incoming field, z direction.");
  params.addParam<FunctionName>("Inc_imag_x", 0.0, "Imaginary incoming field, x direction.");
  params.addParam<FunctionName>("Inc_imag_y", 0.0, "Imaginary incoming field, y direction.");
  params.addParam<FunctionName>("Inc_imag_z", 0.0, "Imaginary incoming field, z direction.");
  return params;
}

VectorPortBC::VectorPortBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),

    _beta(getFunction("beta")),

    _component(getParam<MooseEnum>("component")),

    _coupled_val(coupledVectorValue("coupled")),

    _inc_real_x(getFunction("Inc_real_x")),
    _inc_real_y(getFunction("Inc_real_y")),
    _inc_real_z(getFunction("Inc_real_z")),

    _inc_imag_x(getFunction("Inc_imag_x")),
    _inc_imag_y(getFunction("Inc_imag_y")),
    _inc_imag_z(getFunction("Inc_imag_z"))
{
}

Real
VectorPortBC::computeQpResidual()
{

  RealVectorValue incoming_real(_inc_real_x.value(_t, _q_point[_qp]),
                                _inc_real_y.value(_t, _q_point[_qp]),
                                _inc_real_z.value(_t, _q_point[_qp]));
  RealVectorValue incoming_imag(_inc_imag_x.value(_t, _q_point[_qp]),
                                _inc_imag_y.value(_t, _q_point[_qp]),
                                _inc_imag_z.value(_t, _q_point[_qp]));

  RealVectorValue Q_real = 2 * incoming_imag * _beta.value(_t, _q_point[_qp]);
  RealVectorValue Q_imag = -2 * incoming_real * _beta.value(_t, _q_point[_qp]);

  if (_component == "real")
  {
    return Q_real * _test[_i][_qp] - _beta.value(_t, _q_point[_qp]) *
                                         (_normals[_qp].cross(_test[_i][_qp])) *
                                         (_normals[_qp].cross(_coupled_val[_qp]));
  }
  else
  {
    return Q_imag * _test[_i][_qp] + _beta.value(_t, _q_point[_qp]) *
                                         (_normals[_qp].cross(_test[_i][_qp])) *
                                         (_normals[_qp].cross(_coupled_val[_qp]));
  }
}

Real
VectorPortBC::computeQpJacobian()
{
  return 0.0;
}
