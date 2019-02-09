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
#include "MooseVariableFE.h"
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
    MooseVariableInterface<RealVectorValue>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*mooseVariable()),
    _normals(_var.normals()),
    _phi(_assembly.phiFace(_var)),
    _test(_var.phiFace()),
    _u(_is_implicit ? _var.sln() : _var.slnOld())
{
  addMooseVariableDependency(mooseVariable());
}

void
VectorIntegratedBC::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      Real residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();
      _local_re(_i) += residual;
    }

  accumulateTaggedLocalResidual();
}

void
VectorIntegratedBC::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  accumulateTaggedLocalMatrix();
}

void
VectorIntegratedBC::computeJacobianBlock(MooseVariableFEBase & jvar)
{
  size_t jvar_num = jvar.number();
  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jvar.phiFaceSize(); _j++)
      {
        if (_var.number() == jvar_num)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        else
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar_num);
      }

  accumulateTaggedLocalMatrix();
}

void
VectorIntegratedBC::computeJacobianBlockScalar(unsigned int jvar)
{
  prepareMatrixTag(_assembly, _var.number(), jvar);

  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jv.order(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);

  accumulateTaggedLocalMatrix();
}
