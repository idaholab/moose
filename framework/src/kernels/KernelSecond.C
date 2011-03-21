#include "KernelSecond.h"

template<>
InputParameters validParams<KernelSecond>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


KernelSecond::KernelSecond(const std::string & name, InputParameters parameters):
  Kernel(name, parameters)
{
}

KernelSecond::~KernelSecond()
{
}

void
KernelSecond::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","KernelSecond");
  
  DenseVector<Number> & re = _var.residualBlock();

  _value.resize(_qrule->n_points());
  precalculateResidual();
  for (_i=0; _i<_phi.size(); _i++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      re(_i) += _JxW[_qp]*(_value[_qp]*_second_test[_i][_qp].tr());
  
//  Moose::perf_log.pop("computeResidual()","KernelSecond");
}

Real
KernelSecond::computeQpResidual()
{
  return 0;
}
