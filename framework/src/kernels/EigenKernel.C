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

#include "EigenKernel.h"
#include "MooseEigenSystem.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "EigenExecutionerBase.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<EigenKernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.addParam<bool>(
      "eigen", true, "Use for eigenvalue problem (true) or source problem (false)");
  params.addParam<PostprocessorName>(
      "eigen_postprocessor", 1.0, "The name of the postprocessor that provides the eigenvalue.");
  params.registerBase("EigenKernel");
  return params;
}

EigenKernel::EigenKernel(const InputParameters & parameters)
  : KernelBase(parameters),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _eigen(getParam<bool>("eigen")),
    _eigen_sys(dynamic_cast<MooseEigenSystem *>(&_fe_problem.getNonlinearSystemBase())),
    _eigenvalue(NULL)
{
  // The name to the postprocessor storing the eigenvalue
  std::string eigen_pp_name;

  // If the "eigen_postprocessor" is given, use it. The isParamValid does not work here because of
  // the default value, which
  // you don't want to use if an EigenExecutioner exists.
  if (hasPostprocessor("eigen_postprocessor"))
    eigen_pp_name = getParam<PostprocessorName>("eigen_postprocessor");

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
    _eigenvalue = &getDefaultPostprocessorValue("eigen_postprocessor");

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
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  mooseAssert(*_eigenvalue != 0.0, "Can't divide by zero eigenvalue in EigenKernel!");
  Real one_over_eigen = 1.0 / *_eigenvalue;
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpResidual();

  re += _local_re;

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

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  mooseAssert(*_eigenvalue != 0.0, "Can't divide by zero eigenvalue in EigenKernel!");
  Real one_over_eigen = 1.0 / *_eigenvalue;
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
EigenKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (!_is_implicit)
    return;

  if (jvar == _var.number())
    computeJacobian();
  else
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
    _local_ke.resize(ke.m(), ke.n());
    _local_ke.zero();

    mooseAssert(*_eigenvalue != 0.0, "Can't divide by zero eigenvalue in EigenKernel!");
    Real one_over_eigen = 1.0 / *_eigenvalue;
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          _local_ke(_i, _j) +=
              _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpOffDiagJacobian(jvar);

    ke += _local_ke;
  }
}

bool
EigenKernel::enabled()
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
