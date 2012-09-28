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

#include "KernelGrad.h"
#include "SubProblem.h"
#include "SystemBase.h"

template<>
InputParameters validParams<KernelGrad>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


KernelGrad::KernelGrad(const std::string & name, InputParameters parameters):
    Kernel(name, parameters)
{
}

KernelGrad::~KernelGrad()
{
}

void
KernelGrad::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","KernelGrad");

  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  unsigned int n_qp = _qrule->n_points();
  unsigned int n_test = _test.size();

  for (_qp=0; _qp<n_qp; _qp++)
  {
    _value = precomputeQpResidual();

    Real jxw = _JxW[_qp];
    Real coord = _coord[_qp];

    for (_i=0; _i<n_test; _i++)
      _local_re(_i) += jxw*coord*_value*_grad_test[_i][_qp];
  }

  re += _local_re;

  if(_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
//  Moose::perf_log.pop("computeResidual()","KernelGrad");
}

void
KernelGrad::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  unsigned int n_qp = _qrule->n_points();
  unsigned int n_phi = _phi.size();
  unsigned int n_test = _test.size();

  for (_qp=0; _qp<n_qp; _qp++)
  {
    Real jxw = _JxW[_qp];
    Real coord = _coord[_qp];

    for (_j=0; _j<n_phi; _j++)
    {
      _value = precomputeQpJacobian();

      for (_i=0; _i<n_test; _i++)
        _local_ke(_i, _j) += jxw*coord*_value*_grad_test[_i][_qp];
    }
  }

  ke += _local_ke;

  if(_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for(unsigned int i=0; i<rows; i++)
      diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i<_diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
KernelGrad::computeOffDiagJacobian(unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  DenseMatrix<Number> & Ke = _assembly.jacobianBlock(_var.number(), jvar);

  unsigned int n_qp = _qrule->n_points();
  unsigned int n_phi = _phi.size();
  unsigned int n_test = _test.size();
  unsigned int var_num = _var.number();

  for (_j=0; _j<n_phi; _j++)
    for (_qp=0; _qp<n_qp; _qp++)
    {
      Real off_diag_value;

      Real jxw = _JxW[_qp];
      Real coord = _coord[_qp];

      if(jvar == var_num)
      {
        _value = precomputeQpJacobian();
        for (_i=0; _i<n_test; _i++)
          Ke(_i,_j) += jxw*coord*_value*_grad_test[_i][_qp];
      }
      else
      {
        for (_i=0; _i<n_test; _i++)
        {
          off_diag_value = computeQpOffDiagJacobian(jvar);
          Ke(_i,_j) += jxw*coord*off_diag_value;
        }
      }

    }

//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}

Real
KernelGrad::computeQpResidual()
{
  return 0;
}

RealGradient
KernelGrad::precomputeQpJacobian()
{
  return RealGradient();
}
