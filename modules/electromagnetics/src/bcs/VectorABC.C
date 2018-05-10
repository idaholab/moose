#include "VectorABC.h"

registerMooseObject("ElkApp", VectorABC);

template <>
InputParameters
validParams<VectorABC>()
{
  InputParameters params = validParams<VectorIntegratedBC>();
  params.addClassDescription(
      "First order ABC from Jin 'Theory and Computation of Electromagnetic Fields' by JM Jin.");
  params.addRequiredParam<FunctionName>("beta", "Waveguide propagation constant.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>(
      "component", component, "Variable field component (real or imaginary).");
  params.addRequiredCoupledVar("coupled", "Coupled field variable.");
  return params;
}

VectorABC::VectorABC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),

    _beta(getFunction("beta")),

    _component(getParam<MooseEnum>("component")),

    _coupled_val(coupledVectorValue("coupled"))
{
}

Real
VectorABC::computeQpResidual()
{
  if (_component == "real")
  {
    return -1.0 * _beta.value(_t, _q_point[_qp]) * (_normals[_qp].cross(_test[_i][_qp])) *
           (_normals[_qp].cross(_coupled_val[_qp]));
  }
  else
  {
    return _beta.value(_t, _q_point[_qp]) * (_normals[_qp].cross(_test[_i][_qp])) *
           (_normals[_qp].cross(_coupled_val[_qp]));
  }
}

Real
VectorABC::computeQpJacobian()
{
  return 0.0;
}
