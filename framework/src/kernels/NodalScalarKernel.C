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
  params.addRequiredParam<std::string>("ced_variable", "The name of the variable this kernel is constraining");
  params.addRequiredParam<std::vector<unsigned int> >("nodes", "Node ids");
  return params;
}

NodalScalarKernel::NodalScalarKernel(const std::string & name, InputParameters parameters) :
    ScalarKernel(name, parameters),
    Coupleable(parameters, true),
    _ced_var(_sys.getVariable(_tid, parameters.get<std::string>("ced_variable"))),
    _u_ced(_ced_var.nodalSln()),
    _node_ids(getParam<std::vector<unsigned int> >("nodes"))
{
}

NodalScalarKernel::~NodalScalarKernel()
{
}

void
NodalScalarKernel::reinit()
{
  _problem.reinitNodes(_node_ids, _tid);        // compute variables at nodes
  _assembly.prepareOffDiagScalar();
}

void
NodalScalarKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
}
