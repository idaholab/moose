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

#include "ODEKernel.h"

template<>
InputParameters validParams<ODEKernel>()
{
  InputParameters params = validParams<ScalarKernel>();
  return params;
}

ODEKernel::ODEKernel(const std::string & name, InputParameters parameters) :
    ScalarKernel(name, parameters)
{
}

ODEKernel::~ODEKernel()
{
}

void
ODEKernel::reinit()
{
}

void
ODEKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlockScalar(_var.number());
  for (_i = 0; _i < _var.order(); _i++)
    re(_i) += computeQpResidual();
}

void
ODEKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlockScalar(_var.number(), _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    for (_j = 0; _j < _var.order(); _j++)
    {
      if (_i == _j)
        ke(_i, _j) += computeQpJacobian();
      else
        ke(_i, _j) += computeQpOffDiagJacobian(_j);
    }
}

Real
ODEKernel::computeQpJacobian()
{
  return 0.;
}

Real
ODEKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
