#include "CHInterface.h"

template<>
InputParameters validParams<CHInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("kappa_name","The kappa used with the kernel");
  params.addRequiredParam<std::string>("mob_name","The mobility used with the kernel");
  params.addRequiredParam<std::string>("grad_mob_name","The gradient of the mobility used with the kernel");
  
  return params;
}

CHInterface::CHInterface(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _kappa_name(getParam<std::string>("kappa_name")),
   _mob_name(getParam<std::string>("mob_name")),
   _grad_mob_name(getParam<std::string>("grad_mob_name")),
   _kappa(getMaterialProperty<Real>(_kappa_name)),
   _M(getMaterialProperty<Real>(_mob_name)),
   _grad_M(getMaterialProperty<RealGradient>(_grad_mob_name))
{
}


void
KernelValue::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","KernelGrad");
  
  DenseSubVector<Number> & var_Re = *_dof_data._var_Res[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    _value = precomputeQpResidual();
    for (_i=0; _i<_phi.size(); _i++)
      var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*_value*_test[_i][_qp];
  }
  
//  Moose::perf_log.pop("computeResidual()","KernelGrad");
}

void
KernelValue::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()",_name);

  DenseMatrix<Number> & var_Ke = *_dof_data._var_Kes[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_j=0; _j<_phi.size(); _j++)
    {
      _value = precomputeQpJacobian();
      for (_i=0; _i<_phi.size(); _i++)
        var_Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*_value*_test[_i][_qp];
    }
  }
  
//  Moose::perf_log.pop("computeJacobian()",_name);
}



Real
CHInterface::computeQpResidual()
{
  //Actual value to return
  Real value = 0.0;
  
  value += _kappa[_qp]*(_second_u[_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]));
  
  return value;
}

Real
CHInterface::computeQpJacobian()
{
  //Actual value to return
  Real value = 0.0;

  value += _kappa[_qp]*_second_phi[_j][_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]);
  
  return value;
}
  
