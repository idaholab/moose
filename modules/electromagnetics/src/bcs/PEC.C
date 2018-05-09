#include "PEC.h"

registerMooseObject("ElkApp", PEC);

template <>
InputParameters
validParams<PEC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addClassDescription("NodalNormals system description of the Perfect Electrical Conductor "
                             "(PEC) Boundary Condition, where ${\\hat{n} \\times \\vec{E} = 0}.");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "Component/dimension of the solution variable.");
  params.addCoupledVar("coupled_0", 0.0, "Coupled field variable, 0 component.");
  params.addCoupledVar("coupled_1", 0.0, "Coupled field variable, 1 component.");
  params.addCoupledVar("coupled_2", 0.0, "Coupled field variable, 2 component.");
  params.addRequiredParam<Real>("penalty", "The penalty coefficient.");
  return params;
}

PEC::PEC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _coupled_val_0(coupledValue("coupled_0")),
    _coupled_val_1(coupledValue("coupled_1")),
    _coupled_val_2(coupledValue("coupled_2")),
    _component(getParam<MooseEnum>("component")),
    _penalty(getParam<Real>("penalty"))
{
}

Real
PEC::computeQpResidual()
{
  RealVectorValue field(_coupled_val_0[_qp], _coupled_val_1[_qp], _coupled_val_2[_qp]);
  RealVectorValue test(_test[_i][_qp], _test[_i][_qp], _test[_i][_qp]);

  RealVectorValue NcF = _normals[_qp].cross(field);
  RealVectorValue NcT = _normals[_qp].cross(test);

  int comp = _component - 1;
  if (_component == 0)
    comp = 2;

  // return single component of calculated cross product to be used in the residual calculation.
  return -test * NcF;
}
