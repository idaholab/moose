//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayKernel.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
ArrayKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("ArrayKernel");
  return params;
}

ArrayKernel::ArrayKernel(const InputParameters & parameters)
  : KernelBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _array_grad_test(_var.arrayGradPhi()),
    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.gradPhi(_var)),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _count(_var.count()),
    _work_vector(_count)
{
  addMooseVariableDependency(mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    ArrayMooseVariable * var = &_subproblem.getArrayVariable(_tid, _save_in_strings[i]);

    if (_fe_problem.getNonlinearSystemBase().hasVariable(_save_in_strings[i]))
      paramError("save_in", "cannot use solution variable as save-in variable");

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    ArrayMooseVariable * var = &_subproblem.getArrayVariable(_tid, _diag_save_in_strings[i]);

    if (_fe_problem.getNonlinearSystemBase().hasVariable(_diag_save_in_strings[i]))
      paramError("diag_save_in", "cannot use solution variable as diag save-in variable");

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

void
ArrayKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpResidual();
    for (_i = 0; _i < _test.size(); _i++)
    {
      _work_vector.setZero();
      computeQpResidual(_work_vector);
      mooseAssert(_work_vector.size() == _count,
                  "Size of local residual is not equal to the number of array variable compoments");
      _work_vector *= _JxW[_qp] * _coord[_qp];
      _assembly.saveLocalArrayResidual(_local_re, _i, _test.size(), _work_vector);
    }
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(_local_re);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

void
ArrayKernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  precalculateJacobian();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpJacobian();
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
      {
        _work_vector = computeQpJacobian() * _JxW[_qp] * _coord[_qp];
        _assembly.saveDiagLocalArrayJacobian(
            _local_ke, _i, _test.size(), _j, _phi.size(), _var.number(), _work_vector);
      }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

RealEigenVector
ArrayKernel::computeQpJacobian()
{
  return RealEigenVector::Zero(_var.count());
}

void
ArrayKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);

  bool same_var = (jvar_num == _var.number());

  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting on
  // the displaced mesh
  auto phi_size = jvar.dofIndices().size();

  precalculateOffDiagJacobian(jvar_num);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpOffDiagJacobian(jvar);
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < phi_size; _j++)
      {
        _work_matrix = computeQpOffDiagJacobian(jvar) * _JxW[_qp] * _coord[_qp];
        _assembly.saveFullLocalArrayJacobian(
            _local_ke, _i, _test.size(), _j, phi_size, _var.number(), jvar_num, _work_matrix);
      }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && same_var)
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

RealEigenMatrix
ArrayKernel::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
    return computeQpJacobian().asDiagonal();
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}

void
ArrayKernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  prepareMatrixTag(_assembly, _var.number(), jvar);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      _work_matrix = computeQpOffDiagJacobianScalar(jv) * _JxW[_qp] * _coord[_qp];
      _assembly.saveFullLocalArrayJacobian(
          _local_ke, _i, _test.size(), 0, 1, _var.number(), jvar, _work_matrix);
    }

  accumulateTaggedLocalMatrix();
}

RealEigenMatrix
ArrayKernel::computeQpOffDiagJacobianScalar(const MooseVariableScalar & jvar)
{
  return RealEigenMatrix::Zero(_var.count(), (unsigned int)jvar.order() + 1);
}
