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

#include "AuxNodalScalarKernel.h"
#include "SystemBase.h"

template<>
InputParameters validParams<AuxNodalScalarKernel>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addRequiredParam<std::vector<dof_id_type> >("nodes", "Node ids");
  return params;
}

AuxNodalScalarKernel::AuxNodalScalarKernel(const InputParameters & parameters) :
    AuxScalarKernel(parameters),
    Coupleable(this, true),
    MooseVariableDependencyInterface(this),
    _node_ids(getParam<std::vector<dof_id_type> >("nodes"))
{
  addMooseVariableDependency(getCoupledMooseVars());
}

void
AuxNodalScalarKernel::compute()
{
  _subproblem.reinitNodes(_node_ids, _tid);        // compute variables at nodes
  AuxScalarKernel::compute();
}
