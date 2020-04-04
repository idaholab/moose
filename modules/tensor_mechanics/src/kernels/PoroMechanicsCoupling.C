//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PoroMechanicsCoupling.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", PoroMechanicsCoupling);

InputParameters
PoroMechanicsCoupling::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Adds $-Bi \\cdot p_s \\cdot \\nabla \\Psi_c$, where the subscript $c$ is the component.");
  params.addRequiredCoupledVar("porepressure", "Pore pressure, $p_s$.");
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
