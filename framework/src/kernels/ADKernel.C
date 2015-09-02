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

#include "ADKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/threads.h"

template<>
InputParameters validParams<ADKernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.registerBase("Kernel");
  return params;
}

ADKernel::ADKernel(const InputParameters & parameters) :
    KernelBase(parameters),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
{
}

ADKernel::~ADKernel()
{
}

void
ADKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual().value();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
ADKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  for (_i = 0; _i < _test.size(); _i++)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      ADReal residual = computeQpResidual(); // This will also compute the derivative with respect to all dofs
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * residual.derivatives()[ad_offset + _j];
    }
  }

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

void
ADKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    size_t ad_offset = jvar * _sys.getMaxVarNDofsPerElem();

    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

    for (_i = 0; _i < _test.size(); _i++)
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        ADReal residual = computeQpResidual(); // This will also compute the derivative with respect to all dofs

        for (_j = 0; _j < _phi.size(); _j++)
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * residual.derivatives()[ad_offset + _j];
      }
    }
  }
}

void
ADKernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  /*
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
  */
}
