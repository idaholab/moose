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

#include "ArrayKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ArrayKernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.registerBase("ArrayKernel");
  return params;
}

ArrayKernel::ArrayKernel(const InputParameters & parameters) :
    KernelBase(parameters),
    ArrayMooseVariableInterface(this, false),
    _array_var(dynamic_cast<ArrayMooseVariable &>(_var)),
    _u(_is_implicit ? _array_var.sln() : _array_var.slnOld()),
    _grad_u(_is_implicit ? _array_var.gradSln() : _array_var.gradSlnOld()),
    _u_dot(_array_var.uDot()),
    _du_dot_du(_array_var.duDotDu()),
    _test(_array_var.phi()),
    _grad_test(_array_var.gradPhi()),
    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi())
{
  addMooseVariableDependency(mooseVariable());

  _residual.resize(_array_var.count());
}

void
ArrayKernel::computeResidual()
{
  auto & re = _assembly.arrayResidualBlock(_array_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_i = 0; _i < _test.size(); _i++)
    {
      computeQpResidual();

      // Note: using an external code I have micro-optimized this.
      // This is as close to optimal as can be
      re[_i].noalias() += _JxW[_qp] * _coord[_qp] * _residual;
    }
  }
}

void
ArrayKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_array_var.number(), _array_var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

//  for (_i = 0; _i < _test.size(); _i++)
//    for (_j = 0; _j < _phi.size(); _j++)
//      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
//        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i=0; i<rows; i++)
      diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
ArrayKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _array_var.number())
    computeJacobian();
  else
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_array_var.number(), jvar);

//    for (_i = 0; _i < _test.size(); _i++)
//      for (_j = 0; _j < _phi.size(); _j++)
//        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
//          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
  }
}

void
ArrayKernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_array_var.number(), jvar);
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

//  for (_i = 0; _i < _test.size(); _i++)
//    for (_j = 0; _j < jv.order(); _j++)
//      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
//        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
}

void
ArrayKernel::computeQpJacobian()
{
}

void
ArrayKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
}

void
ArrayKernel::precalculateResidual()
{
}
