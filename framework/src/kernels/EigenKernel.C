//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "EigenExecutionerBase.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "MooseEigenSystem.h"
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

InputParameters
EigenKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<bool>(
      "eigen", true, "Use for eigenvalue problem (true) or source problem (false)");
  params.addParam<PostprocessorName>(
      "eigen_postprocessor", 1.0, "The name of the postprocessor that provides the eigenvalue.");
  params.registerBase("EigenKernel");
  return params;
}

EigenKernel::EigenKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _eigen(getParam<bool>("eigen")),
    _eigen_sys(dynamic_cast<MooseEigenSystem *>(&_fe_problem.getNonlinearSystemBase())),
    _eigenvalue(NULL)
{
  // The name to the postprocessor storing the eigenvalue
  std::string eigen_pp_name;

  // If the "eigen_postprocessor" is given, use it. The isParamValid does not work here because of
  // the default value, which
  // you don't want to use if an EigenExecutioner exists.
  if (!isDefaultPostprocessorValue("eigen_postprocessor"))
    eigen_pp_name = getPostprocessorName("eigen_postprocessor");

  // Attempt to extract the eigenvalue postprocessor from the Executioner
  else
  {
    EigenExecutionerBase * exec = dynamic_cast<EigenExecutionerBase *>(_app.getExecutioner());
    if (exec)
      eigen_pp_name = exec->getParam<PostprocessorName>("bx_norm");
  }

  // If the postprocessor name was not provided and an EigenExecutionerBase is not being used,
  // use the default value from the "eigen_postprocessor" parameter
  if (eigen_pp_name.empty())
    _eigenvalue = &getPostprocessorValue("eigen_postprocessor");

  // If the name does exist, then use the postprocessor value
  else
  {
    if (_is_implicit)
      _eigenvalue = &getPostprocessorValueByName(eigen_pp_name);
    else
    {
      EigenExecutionerBase * exec = dynamic_cast<EigenExecutionerBase *>(_app.getExecutioner());
      if (exec)
        _eigenvalue = &exec->eigenvalueOld();
      else
        _eigenvalue = &getPostprocessorValueOldByName(eigen_pp_name);
    }
  }
}

void
EigenKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();

  mooseAssert(*_eigenvalue != 0.0, "Can't divide by zero eigenvalue in EigenKernel!");
  Real one_over_eigen = 1.0 / *_eigenvalue;
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpResidual();

  accumulateTaggedLocalResidual();
  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

void
EigenKernel::computeJacobian()
{
  if (!_is_implicit)
    return;

  prepareMatrixTag(_assembly, _var.number(), _var.number());

  precalculateJacobian();
  mooseAssert(*_eigenvalue != 0.0, "Can't divide by zero eigenvalue in EigenKernel!");
  Real one_over_eigen = 1.0 / *_eigenvalue;
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpJacobian();

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
EigenKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (!_is_implicit)
    return;

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    const auto & jvar = getVariable(jvar_num);

    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = jvar.dofIndices().size();
    mooseAssert(
        phi_size * jvar.count() == _local_ke.n(),
        "The size of the phi container does not match the number of local Jacobian columns");

    if (_local_ke.m() != _test.size())
      return;

    precalculateOffDiagJacobian(jvar_num);
    mooseAssert(*_eigenvalue != 0.0, "Can't divide by zero eigenvalue in EigenKernel!");
    Real one_over_eigen = 1.0 / *_eigenvalue;
    if (jvar.count() == 1)
    {
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < phi_size; _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _local_ke(_i, _j) +=
                _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpOffDiagJacobian(jvar.number());
    }
    else
    {
      unsigned int n = phi_size;
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < n; _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          {
            RealEigenVector v = _JxW[_qp] * _coord[_qp] * one_over_eigen *
                                computeQpOffDiagJacobianArray(static_cast<ArrayMooseVariable &>(
                                    const_cast<MooseVariableFieldBase &>(jvar)));
            for (unsigned int k = 0; k < v.size(); ++k)
              _local_ke(_i, _j + k * n) += v(k);
          }
    }

    accumulateTaggedLocalMatrix();
  }
}

bool
EigenKernel::enabled() const
{
  bool flag = MooseObject::enabled();
  if (_eigen)
  {
    if (_is_implicit)
      return flag && (!_eigen_sys->activeOnOld());
    else
      return flag && _eigen_sys->activeOnOld();
  }
  else
    return flag;
}
