//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementUserObject.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "Assembly.h"

#include "libmesh/elem.h"

template <>
InputParameters
validParams<ElementUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<MaterialPropertyInterface>();
  params += validParams<TransientInterface>();
  params += validParams<RandomInterface>();
  return params;
}

ElementUserObject::ElementUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BlockRestrictable(this),
    MaterialPropertyInterface(this, blockIDs()),
    UserObjectInterface(this),
    Coupleable(this, false),
    MooseVariableDependencyInterface(),
    TransientInterface(this),
    PostprocessorInterface(this),
    RandomInterface(parameters, _fe_problem, _tid, false),
    _mesh(_subproblem.mesh()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}
