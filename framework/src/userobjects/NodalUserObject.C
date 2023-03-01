//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalUserObject.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

InputParameters
NodalUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params.addParam<bool>("unique_node_execute",
                        false,
                        "When false (default), block restricted objects will have the "
                        "execute method called multiple times on a single node if the "
                        "node lies on a interface between two subdomains.");
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();
  params += RandomInterface::validParams();
  return params;
}

NodalUserObject::NodalUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BlockRestrictable(this),
    BoundaryRestrictable(this, blockIDs(), true), // true for applying to nodesets
    Coupleable(this, true),
    MooseVariableDependencyInterface(this),
    TransientInterface(this),
    RandomInterface(parameters, _fe_problem, _tid, true),
    _mesh(_subproblem.mesh()),
    _qp(0),
    _current_node(_assembly.node()),
    _unique_node_execute(getParam<bool>("unique_node_execute"))
{
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
NodalUserObject::subdomainSetup()
{
  mooseError("NodalUserObjects do not execute subdomainSetup method, this function does nothing "
             "and should not be used.");
}
