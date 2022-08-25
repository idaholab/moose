//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrPolyLinearizedInterface.h"

#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", ACGrGrPolyLinearizedInterface);

InputParameters
ACGrGrPolyLinearizedInterface::validParams()
{
  InputParameters params = ACGrGrPoly::validParams();
  params.addClassDescription(
      "Grain growth model Allen-Cahn Kernel with linearized interface variable transformation");
  params.addRequiredParam<MaterialPropertyName>(
      "this_op", "The material property defining the order parameter for this variable");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "other_ops", "List of properties defining the order parameters for the variables in v");
  return params;
}

ACGrGrPolyLinearizedInterface::ACGrGrPolyLinearizedInterface(const InputParameters & parameters)
  : ACGrGrPoly(parameters),
    _other_op_names(getParam<std::vector<MaterialPropertyName>>("other_ops")),
    _num_ops(_other_op_names.size()),
    _gamma(getMaterialProperty<Real>("gamma_asymm")),
    _op(getMaterialProperty<Real>("this_op")),
    _dopdphi(getMaterialPropertyDerivative<Real>("this_op", _var.name())),
    _opj(_num_ops),
    _dopjdarg(_num_ops)
{
  if (_num_ops != _op_num)
    mooseError(
        "In ACGrGrPolyLinearizedInterface, number of coupled variables in v must match number of "
        "coupled materials in other_ops");

  // Iterate over all coupled order parameters
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _opj[i] = &getMaterialPropertyByName<Real>(_other_op_names[i]);
    _dopjdarg[i] = &getMaterialPropertyDerivative<Real>(_other_op_names[i], i);
  }
}

Real
ACGrGrPolyLinearizedInterface::assignThisOp()
{
  return _op[_qp];
}

std::vector<Real>
ACGrGrPolyLinearizedInterface::assignOtherOps()
{
  std::vector<Real> other_ops(_op_num);
  for (unsigned int i = 0; i < _op_num; ++i)
    other_ops[i] = (*_opj[i])[_qp];

  return other_ops;
}

Real
ACGrGrPolyLinearizedInterface::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return ACGrGrPoly::computeDFDOP(type);

    case Jacobian:
      return ACGrGrPoly::computeDFDOP(type) * _dopdphi[_qp];

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrPolyLinearizedInterface::computeQpOffDiagJacobian(unsigned int jvar)
{

  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
      return ACGrGrPoly::computeQpOffDiagJacobian(jvar) * (*_dopjdarg[i])[_qp];

  return 0.0;
}
