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

template<>
InputParameters validParams<NodalScalarKernel>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addRequiredParam<NonlinearVariableName>("ced_variable", "The name of the variable this kernel is constraining");
  params.addRequiredParam<std::vector<unsigned int> >("nodes", "Node ids");
  return params;
}

NodalScalarKernel::NodalScalarKernel(const std::string & name, InputParameters parameters) :
    ScalarKernel(name, parameters),
    Coupleable(parameters, true),
    MooseVariableDependencyInterface(),
    _ced_var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("ced_variable"))),
    _u_ced(_ced_var.nodalSln()),
    _node_ids(getParam<std::vector<unsigned int> >("nodes"))
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for(unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

NodalScalarKernel::~NodalScalarKernel()
{
}

void
NodalScalarKernel::reinit()
{
  _subproblem.reinitNodes(_node_ids, _tid);        // compute variables at nodes
  _assembly.prepareOffDiagScalar();
}

void
NodalScalarKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.index())
    computeJacobian();
}
