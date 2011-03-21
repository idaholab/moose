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

#include "IntegratedBC.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"


template<>
InputParameters validParams<IntegratedBC>()
{
  return validParams<BoundaryCondition>();
}


IntegratedBC::IntegratedBC(const std::string & name, InputParameters parameters) :
    BoundaryCondition(name, parameters),
    Coupleable(parameters, false),
    _test_var(_problem.getVariable(0, parameters.get<std::string>("variable"))),

    _qrule(_subproblem.qRuleFace(_tid)),
    _q_point(_subproblem.pointsFace(_tid)),
    _JxW(_subproblem.JxWFace(_tid)),

    _phi(_var.phiFace()),
    _grad_phi(_var.gradPhiFace()),
    _second_phi(_var.secondPhi()),

    _test(_test_var.testFace()),
    _grad_test(_test_var.gradTestFace()),
    _second_test(_var.secondTest()),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _second_u(_var.secondSln())
{
}

IntegratedBC::~IntegratedBC()
{
}

void
IntegratedBC::computeResidual()
{
  DenseVector<Number> &re = _var.residualBlock();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _phi.size(); _i++)
    {
      re(_i) += _JxW[_qp]*computeQpResidual();
    }
}

void
IntegratedBC::computeJacobian(int /*i*/, int /*j*/)
{
  DenseMatrix<Number> & ke = _var.jacobianBlock();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_i = 0; _i < _phi.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        ke(_i, _j) += _JxW[_qp]*computeQpJacobian();
  }
}

void
IntegratedBC::computeJacobianBlock(DenseMatrix<Number> & Ke, unsigned int ivar, unsigned int jvar)
{
//  Moose::perf_log.push("computeJacobianBlock()","IntegratedBC");

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
      {
        if (ivar == jvar)
          Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
        else
          Ke(_i,_j) += _JxW[_qp]*computeQpOffDiagJacobian(jvar);
      }

//  Moose::perf_log.pop("computeJacobianBlock()","IntegratedBC");
}

Real
IntegratedBC::computeQpJacobian()
{
  return 0;
}

Real
IntegratedBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}
