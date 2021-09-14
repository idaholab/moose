//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialVectorBodyForce.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", MaterialVectorBodyForce);

InputParameters
MaterialVectorBodyForce::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Apply a body force vector to the coupled displacement component.");
  params.addParam<FunctionName>(
      "function", "1", "Function to scale the coupled body force vector property");
  params.addParam<Real>(
      "hht_alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  params.addRequiredParam<MaterialPropertyName>("body_force", "Force per unit volume vector");
  params.addCoupledVar("displacements", "The displacements");
  return params;
}

MaterialVectorBodyForce::MaterialVectorBodyForce(const InputParameters & parameters)
  : Kernel(parameters),
    _component(libMesh::invalid_uint),
    _body_force(getMaterialProperty<RealVectorValue>("body_force")),
    _function(getFunction("function")),
    _alpha(getParam<Real>("hht_alpha"))
{
  for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
    if (_var.number() == getVar("displacements", i)->number())
      _component = i;

  if (_component == libMesh::invalid_uint)
    this->paramError("variable",
                     "The kernel variable needs to be one of the 'displacements' variables");
}

Real
MaterialVectorBodyForce::computeQpResidual()
{
  Real factor = _function.value(_t + _alpha * _dt, _q_point[_qp]);
  return -_body_force[_qp](_component) * _test[_i][_qp] * factor;
}
