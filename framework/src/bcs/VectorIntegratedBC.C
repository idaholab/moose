/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "VectorIntegratedBC.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableField.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<VectorIntegratedBC>()
{
  InputParameters params = validParams<IntegratedBCBase>();
  params += validParams<RandomInterface>();
  params += validParams<MaterialPropertyInterface>();
  return params;
}

VectorIntegratedBC::VectorIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<RealVectorValue>(this, false),
    _var(*mooseVariable()),
    _normals(_var.normals()),
    _phi(_assembly.phiFace(_var)),
    _curl_phi(_assembly.curlPhiFace(_var)),
    _test(_var.phiFace()),
    _curl_test(_var.curlPhiFace()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _curl_u(_is_implicit ? _var.curlSln() : _var.curlSlnOld())
{
  addMooseVariableDependency(mooseVariable());
}

void
VectorIntegratedBC::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      Real residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();
      _local_re(_i) += residual;
    }

  re += _local_re;
}

void
VectorIntegratedBC::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  ke += _local_ke;
}

void
VectorIntegratedBC::computeJacobianBlock(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
      {
        if (_var.number() == jvar)
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        else
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
      }
}

void
VectorIntegratedBC::computeJacobianBlockScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jv.order(); _j++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
}
