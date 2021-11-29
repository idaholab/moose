//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassLumpedTimeDerivative.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", MassLumpedTimeDerivative);

InputParameters
MassLumpedTimeDerivative::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription(
      "Lumped formulation of the time derivative $\\frac{\\partial u}{\\partial t}$. Its "
      "corresponding weak form is $\\dot{u_i}(\\psi_i, 1)$ where $\\dot{u_i}$ denotes the time "
      "derivative of the solution coefficient associated with node $i$.");
  return params;
}

MassLumpedTimeDerivative::MassLumpedTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters), _u_dot_nodal(_var.dofValuesDot())
{
}

Real
MassLumpedTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _u_dot_nodal[_i];
}

Real
MassLumpedTimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp] * _du_dot_du[_qp];
}

void
MassLumpedTimeDerivative::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      ke(_i, _i) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
}
