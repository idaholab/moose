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
#include "Moose.h"

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
  
  DenseVector<Number> & re = _var.residualBlock();

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    _value = precomputeQpResidual();
    for (_i=0; _i<_phi.size(); _i++)
      re(_i) += _JxW[_qp]*_value*_grad_test[_i][_qp];
  }
  
//  Moose::perf_log.pop("computeResidual()","KernelGrad");
}

void
KernelGrad::computeJacobian(int /*i*/, int /*j*/)
{
//  Moose::perf_log.push("computeJacobian()",_name);

  DenseMatrix<Number> & ke = _var.jacobianBlock();

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_j=0; _j<_phi.size(); _j++)
    {
      _value = precomputeQpJacobian();
      for (_i=0; _i<_phi.size(); _i++)
        ke(_i,_j) += _JxW[_qp]*_value*_grad_test[_i][_qp];
    }
  }
  
//  Moose::perf_log.pop("computeJacobian()",_name);
}

void
KernelGrad::computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  for (_j=0; _j<_phi.size(); _j++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
    {
      Real off_diag_value;
      
      if(jvar == _var.number())
        _value = precomputeQpJacobian();
      else
        off_diag_value = computeQpOffDiagJacobian(jvar);
      
      for (_i=0; _i<_phi.size(); _i++)
      {
        if(jvar == _var.number())
          Ke(_i,_j) += _JxW[_qp]*_value*_grad_test[_i][_qp];
        else
          Ke(_i,_j) += _JxW[_qp]*off_diag_value;
      }
    }

//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}

Real
KernelGrad::computeQpResidual()
{
  return 0;
}
