#include "KernelSecond.h"
#include "Moose.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<KernelSecond>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


KernelSecond::KernelSecond(std::string name, MooseSystem & moose_system, InputParameters parameters):
  Kernel(name, moose_system, parameters)
{
}

KernelSecond::~KernelSecond()
{
}

void
KernelSecond::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","KernelSecond");
  
  DenseSubVector<Number> & var_Re = *_dof_data._var_Res[_var_num];

  _value.resize(_qrule->n_points());
  precalculateResidual();
  for (_i=0; _i<_phi.size(); _i++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*(_value[_qp]*_second_test[_i][_qp].tr());
  
//  Moose::perf_log.pop("computeResidual()","KernelSecond");
}

Real
KernelSecond::computeQpResidual()
{
  return 0;
}
