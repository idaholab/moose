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

#include "KernelSecond.h"
#include "SubProblem.h"
#include "SystemBase.h"

template<>
InputParameters validParams<KernelSecond>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


KernelSecond::KernelSecond(const std::string & name, InputParameters parameters):
    Kernel(name, parameters),
    _second_test(_var.secondPhi())
{
}

KernelSecond::~KernelSecond()
{
}

void
KernelSecond::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  _value.resize(_qrule->n_points());
  precalculateResidual();
  for (_i=0; _i<_test.size(); _i++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp]*_coord[_qp]*(_value[_qp]*_second_test[_i][_qp].tr());

  re += _local_re;

  if(_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

Real
KernelSecond::computeQpResidual()
{
  return 0;
}
