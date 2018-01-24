//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ALEKernel.h"

// MOOSE includes
#include "MooseVariable.h"

template <>
InputParameters
validParams<ALEKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Sets up derivetives with respect to initial configuration");
  return params;
}

ALEKernel::ALEKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _assembly_undisplaced(_fe_problem.assembly(_tid)),
    _var_undisplaced(
        _fe_problem.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _grad_phi_undisplaced(_assembly_undisplaced.gradPhi()),
    _grad_test_undisplaced(_var_undisplaced.gradPhi())
{
}

void
ALEKernel::computeJacobian()
{
  _fe_problem.prepareShapes(_var.number(), _tid);
  Kernel::computeJacobian();
}

void
ALEKernel::computeOffDiagJacobian(unsigned int jvar)
{
  _fe_problem.prepareShapes(jvar, _tid);
  Kernel::computeOffDiagJacobian(jvar);
}
