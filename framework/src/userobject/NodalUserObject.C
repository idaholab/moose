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

#include "NodalUserObject.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

template <>
InputParameters
validParams<NodalUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params.addParam<bool>("unique_node_execute",
                        false,
                        "When false (default), block restricted objects will have the "
                        "execute method called multiple times on a single node if the "
                        "node lies on a interface between two subdomains.");
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<RandomInterface>();
  return params;
}

NodalUserObject::NodalUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters, blockIDs(), true), // true for applying to nodesets
    UserObjectInterface(this),
    Coupleable(this, true),
    MooseVariableDependencyInterface(),
    TransientInterface(this),
    PostprocessorInterface(this),
    RandomInterface(parameters, _fe_problem, _tid, true),
    ZeroInterface(parameters),
    _mesh(_subproblem.mesh()),
    _qp(0),
    _current_node(_assembly.node()),
    _unique_node_execute(getParam<bool>("unique_node_execute"))
{
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
NodalUserObject::subdomainSetup()
{
  mooseError("NodalUserObjects do not execute subdomainSetup method, this function does nothing "
             "and should not be used.");
}
