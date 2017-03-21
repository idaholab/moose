/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PoroMechanicsCoupling.h"
#include "Function.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<PoroMechanicsCoupling>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Adds -BiotCoefficient*porepressure*grad_test[component]");
  params.addRequiredCoupledVar(
      "porepressure",
      "Porepressure.  This kernel adds -BiotCoefficient*porepressure*grad_test[component]");
  params.addRequiredParam<unsigned int>("component",
                                        "The gradient direction (0 for x, 1 for y and 2 for z)");
  return params;
}

PoroMechanicsCoupling::PoroMechanicsCoupling(const InputParameters & parameters)
  : Kernel(parameters),
    _coefficient(getMaterialProperty<Real>("biot_coefficient")),
    _porepressure(coupledValue("porepressure")),
    _porepressure_var_num(coupled("porepressure")),
    _component(getParam<unsigned int>("component"))
{
  if (_component >= _mesh.dimension())
    mooseError("PoroMechanicsCoupling: component should not be greater than the mesh dimension\n");
}

Real
PoroMechanicsCoupling::computeQpResidual()
{
  return -_coefficient[_qp] * _porepressure[_qp] * _grad_test[_i][_qp](_component);
}

Real
PoroMechanicsCoupling::computeQpJacobian()
{
  if (_var.number() != _porepressure_var_num)
    return 0.0;
  return -_coefficient[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](_component);
}

Real
PoroMechanicsCoupling::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _porepressure_var_num)
    return 0.0;
  return -_coefficient[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](_component);
}
