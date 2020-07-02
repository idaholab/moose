//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalAuxVariableUserObjectBasePD.h"
#include "AuxiliarySystem.h"
#include "PeridynamicsMesh.h"
#include "MooseVariable.h"

InputParameters
NodalAuxVariableUserObjectBasePD::validParams()
{
  InputParameters params = ElementUserObjectBasePD::validParams();
  params.addClassDescription("Base class for computing value for nodal AuxVariable from elemental "
                             "information in a peridynamic model");

  params.addRequiredCoupledVar("variable", "Name of AuxVariable this userobject is acting on");

  return params;
}

NodalAuxVariableUserObjectBasePD::NodalAuxVariableUserObjectBasePD(
    const InputParameters & parameters)
  : ElementUserObjectBasePD(parameters), _var(getVar("variable", 0))
{
}

void
NodalAuxVariableUserObjectBasePD::initialize()
{
  std::vector<std::string> zero_vars;
  zero_vars.push_back(_var->name());
  _aux.zeroVariables(zero_vars);

  _aux.solution().close();
}

void
NodalAuxVariableUserObjectBasePD::execute()
{
  for (unsigned int i = 0; i < _nnodes; ++i)
  {
    dof_id_type dof = _current_elem->node_ptr(i)->dof_number(_aux.number(), _var->number(), 0);

    computeValue(i, dof);
  }
}

void
NodalAuxVariableUserObjectBasePD::finalize()
{
  _aux.solution().close();
}
