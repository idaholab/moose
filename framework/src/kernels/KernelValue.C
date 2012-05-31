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
//  Moose::perf_log.push("computeResidual()","KernelGrad");

  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _value = precomputeQpResidual();
    for (_i = 0; _i < _test.size(); _i++)
      re(_i) += _JxW[_qp] * _coord[_qp] * _value * _test[_i][_qp];
  }

//  Moose::perf_log.pop("computeResidual()","KernelGrad");
}

void
KernelValue::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()",_name);

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_j = 0; _j < _phi.size(); _j++)
    {
      // NOTE: is it possible to move this out of the for-loop and multiply the _value by _phi[_j][_qp]
      _value = precomputeQpJacobian();
      for (_i = 0; _i < _test.size(); _i++)
        ke(_i, _j) += _JxW[_qp]*_coord[_qp]*_value*_test[_i][_qp];
    }
  }

//  Moose::perf_log.pop("computeJacobian()",_name);
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
