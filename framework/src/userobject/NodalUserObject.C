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

template<>
InputParameters validParams<NodalUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params.addParam<bool>("unique_block_execute", false, "When false (default), block restricted objects will have the execute method called multiple times on a single node if the node lies on a interface between two subdomains.");
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<RandomInterface>();
  params += validParams<MaterialPropertyInterface>();
  return params;
}

NodalUserObject::NodalUserObject(const InputParameters & parameters) :
    UserObject(parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters, blockIDs()),
    MaterialPropertyInterface(parameters, blockIDs(), boundaryIDs()),
    UserObjectInterface(parameters),
    Coupleable(parameters, true),
    ScalarCoupleable(parameters),
    MooseVariableDependencyInterface(),
    TransientInterface(parameters, "nodal_user_objects"),
    PostprocessorInterface(parameters),
    RandomInterface(parameters, _fe_problem, _tid, true),
    ZeroInterface(parameters),
    _mesh(_subproblem.mesh()),
    _qp(0),
    _current_node(_assembly.node()),
    _unique_block_execute(getParam<bool>("unique_block_execute"))
{
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}


void
NodalUserObject::subdomainSetup()
{
  mooseError("NodalUserObjects do not execute subdomainSetup method, this function does nothing and should not be used.");
}
