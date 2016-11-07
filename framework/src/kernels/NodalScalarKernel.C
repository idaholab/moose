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

#include "NodalScalarKernel.h"
#include "SystemBase.h"
#include "Assembly.h"

template <>
InputParameters
validParams<NodalScalarKernel>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addRequiredParam<std::vector<dof_id_type>>("nodes", "Node ids");
  return params;
}

NodalScalarKernel::NodalScalarKernel(const InputParameters & parameters)
  : ScalarKernel(parameters),
    Coupleable(this, true),
    MooseVariableDependencyInterface(),
    _node_ids(getParam<std::vector<dof_id_type>>("nodes"))
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
NodalScalarKernel::reinit()
{
  _subproblem.reinitNodes(_node_ids, _tid); // compute variables at nodes
  _assembly.prepareOffDiagScalar();
}

void
NodalScalarKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
}
