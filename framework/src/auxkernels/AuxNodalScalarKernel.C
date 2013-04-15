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
  params.addRequiredParam<std::vector<unsigned int> >("nodes", "Node ids");
  return params;
}

AuxNodalScalarKernel::AuxNodalScalarKernel(const std::string & name, InputParameters parameters) :
    AuxScalarKernel(name, parameters),
    Coupleable(parameters, true),
    MooseVariableDependencyInterface(),
    _node_ids(getParam<std::vector<unsigned int> >("nodes"))
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for(unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

AuxNodalScalarKernel::~AuxNodalScalarKernel()
{
}

void
AuxNodalScalarKernel::compute()
{
  _subproblem.reinitNodes(_node_ids, _tid);        // compute variables at nodes
  AuxScalarKernel::compute();
}
