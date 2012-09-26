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

#include "KernelValue.h"
#include "SubProblem.h"
#include "SystemBase.h"

template<>
InputParameters validParams<KernelValue>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


KernelValue::KernelValue(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{
}

KernelValue::~KernelValue()
{
}

void
KernelValue::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _value = precomputeQpResidual();
    for (_i = 0; _i < _test.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * _value * _test[_i][_qp];
  }

  re += _local_re;

  if(_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
KernelValue::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_j = 0; _j < _phi.size(); _j++)
    {
      // NOTE: is it possible to move this out of the for-loop and multiply the _value by _phi[_j][_qp]
      _value = precomputeQpJacobian();
      for (_i = 0; _i < _test.size(); _i++)
        _local_ke(_i, _j) += _JxW[_qp]*_coord[_qp]*_value*_test[_i][_qp];
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
KernelValue::computeOffDiagJacobian(unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  DenseMatrix<Number> & Ke = _assembly.jacobianBlock(_var.number(), jvar);

  for (_j=0; _j<_phi.size(); _j++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
    {
      if(jvar == _var.number())
      {
        _value = _coord[_qp]*precomputeQpJacobian();
        for (_i=0; _i<_test.size(); _i++)
          Ke(_i,_j) += _JxW[_qp]*_coord[_qp]*_value*_test[_i][_qp];
      }
      else
      {
        for (_i=0; _i<_test.size(); _i++)
        {
          _value = _coord[_qp]*computeQpOffDiagJacobian(jvar);
          Ke(_i,_j) += _JxW[_qp]*_coord[_qp]*_value;
        }
      }
    }

//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}

Real
KernelValue::computeQpResidual()
{
  return 0;
}
