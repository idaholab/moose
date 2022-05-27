//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableDependencyInterface.h"
#include "MooseVariableFieldBase.h"
#include "MooseObject.h"
#include "SystemBase.h"
#include "MooseError.h"

#include "libmesh/dof_object.h"
#include "libmesh/dof_map.h"

using namespace libMesh;

MooseVariableDependencyInterface::MooseVariableDependencyInterface(const MooseObject * const) {}

template <typename DofObjectType>
std::set<MooseVariableFieldBase *>
MooseVariableDependencyInterface::checkVariables(
    const DofObjectType & dof_object, const std::set<MooseVariableFieldBase *> & vars_to_check)
{
  std::set<MooseVariableFieldBase *> vars_without_indices;
  for (auto * const var : vars_to_check)
  {
    var->sys().dofMap().dof_indices(&dof_object, _dof_indices, var->number());
    if (_dof_indices.empty())
      vars_without_indices.insert(var);
  }

  return vars_without_indices;
}

template std::set<MooseVariableFieldBase *> MooseVariableDependencyInterface::checkVariables(
    const Elem & dof_object, const std::set<MooseVariableFieldBase *> & vars_to_check);
template std::set<MooseVariableFieldBase *> MooseVariableDependencyInterface::checkVariables(
    const Node & dof_object, const std::set<MooseVariableFieldBase *> & vars_to_check);
