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

MooseVariableDependencyInterface::MooseVariableDependencyInterface(
    const MooseObject * const moose_object)
  : _mvdi_name(moose_object->name()), _mvdi_type(moose_object->type())
{
}

void
MooseVariableDependencyInterface::checkVariables(const libMesh::DofObject & dof_object,
                                                 const bool block,
                                                 const std::string & geometric_name) const
{
  for (const auto * const var : _moose_variable_dependencies)
    if (!dof_object.n_dofs(var->sys().number(), var->number()))
      mooseError("'",
                 _mvdi_name,
                 "' of type '",
                 _mvdi_type,
                 "' depends on variable '",
                 var->name(),
                 "'. However, that variable does not appear to be defined on ",
                 block ? "block" : "boundary",
                 " '",
                 geometric_name,
                 "'.");
}
