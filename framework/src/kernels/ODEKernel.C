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
#include "SystemBase.h"

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
  DenseVector<Number> & re = _assembly.residualBlock(_var.index());
  for (_i = 0; _i < _var.order(); _i++)
    re(_i) += computeQpResidual();
}

void
ODEKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());

  for (_i = 0; _i < _var.order(); _i++)
    for (_j = 0; _j < _var.order(); _j++)
    {
      if (_i == _j)
        ke(_i, _j) += computeQpJacobian();
      else
        ke(_i, _j) += computeQpOffDiagJacobian(_var.index());
    }
}

void
ODEKernel::computeOffDiagJacobian(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), jvar);
  if (_sys.isScalarVariable(jvar))
  {
    MooseVariableScalar & var_j = _sys.getScalarVariable(_tid, jvar);
    for (_i = 0; _i < _var.order(); _i++)
      for (_j = 0; _j < var_j.order(); _j++)
      {
        if (jvar == _var.index())
        {
          if (_i == _j)
            ke(_i, _j) += computeQpJacobian();
          else
            ke(_i, _j) += computeQpOffDiagJacobian(_var.index());
        }
        else
          ke(_i, _j) += computeQpOffDiagJacobian(jvar);
      }
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
