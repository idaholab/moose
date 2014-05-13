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
#include "EigenSystem.h"

template<>
InputParameters validParams<EigenKernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.addParam<bool>("eigen", true, "Use for eigenvalue problem (true) or source problem (false)");
  params.registerBase("EigenKernel");
  return params;
}

EigenKernel::EigenKernel(const std::string & name, InputParameters parameters) :
    KernelBase(name,parameters),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _eigen(getParam<bool>("eigen")),
    _eigen_sys(NULL),
    _eigenvalue(NULL)
{
  if (_eigen)
  {
    _eigen_sys = static_cast<EigenSystem *>(&_fe_problem.getNonlinearSystem());
    _eigen_pp = _fe_problem.parameters().get<PostprocessorName>("eigen_postprocessor");
    if (_is_implicit)
      _eigenvalue = &getPostprocessorValueByName(_eigen_pp);
    else
      _eigenvalue = &getPostprocessorValueOldByName(_eigen_pp);
  }
  else
  {
    if (!_fe_problem.parameters().isParamValid("eigenvalue"))
      _fe_problem.parameters().set<Real>("eigenvalue") = 1.0;
    _eigenvalue = &_fe_problem.parameters().get<Real>("eigenvalue");
  }
}

void
EigenKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  Real one_over_eigen = 1.0;
  one_over_eigen /= *_eigenvalue;
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
EigenKernel::computeJacobian()
{
  if (!_is_implicit) return;

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  Real one_over_eigen = 1.0;
  one_over_eigen /= *_eigenvalue;
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i=0; i<rows; i++)
      diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

Real
EigenKernel::computeQpJacobian()
{
  return 0;
}

bool
EigenKernel::isActive()
{
  bool flag = TransientInterface::isActive();
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
