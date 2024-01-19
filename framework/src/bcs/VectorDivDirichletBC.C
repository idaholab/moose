//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorDivDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorDivDirichletBC);

InputParameters
VectorDivDirichletBC::validParams()
{
  InputParameters params = VectorIntegratedBC::validParams();
  params.addParam<FunctionName>("function",
                                "The boundary condition vector function, "
                                "use as an alternative to a component-wise specification");
  params.addParam<FunctionName>("function_x", 0, "The function for the x component");
  params.addParam<FunctionName>("function_y", 0, "The function for the y component");
  params.addParam<FunctionName>("function_z", 0, "The function for the z component");
  params.addClassDescription("Enforces, in a strong sense, a Dirichlet boundary condition on the "
                             "divergence of a nonlinear vector variable.");
  return params;
}

VectorDivDirichletBC::VectorDivDirichletBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z"))
{
}

void
VectorDivDirichletBC::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  DenseVector<Number> _local_rm(_local_re.size());
  for (_i = 0; _i < _test.size(); _i++)
    _local_rm(_i) = _i != _current_side;

  _i = _current_side;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  maskTaggedLocalResidual(_local_rm);
  accumulateTaggedLocalResidual();
}

void
VectorDivDirichletBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);

  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting on
  // the displaced mesh
  auto phi_size = jvar.dofIndices().size();

  DenseMatrix<Number> _local_km(_local_ke.m(), _local_ke.n());
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < phi_size; _j++)
      _local_km(_i, _j) = _i != _current_side;

  _i = _current_side;
  if (_var.number() == jvar_num)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_j = 0; _j < phi_size; _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  maskTaggedLocalMatrix(_local_km);
  accumulateTaggedLocalMatrix();
}

Real
VectorDivDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact;
  if (_function)
    u_exact = _function->vectorValue(_t, _q_point[_qp]);
  else
    u_exact = {_function_x.value(_t, _q_point[_qp]),
               _function_y.value(_t, _q_point[_qp]),
               _function_z.value(_t, _q_point[_qp])};

  return (_u[_qp] - u_exact) * _normals[_qp];
}

Real
VectorDivDirichletBC::computeQpJacobian()
{
  return _phi[_j][_qp] * _normals[_qp];
}
