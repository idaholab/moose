//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableMappingBase.h"

InputParameters
VariableMappingBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += RestartableModelInterface::validParams();
  params.addClassDescription("Base class for mapping objects.");
  params.registerBase("VariableMappingBase");
  params.registerSystemAttributeName("VariableMappingBase");
  params.addParam<std::vector<VariableName>>("variables",
                                             "The names of the variables which need a mapping.");
  return params;
}

VariableMappingBase::VariableMappingBase(const InputParameters & parameters)
  : MooseObject(parameters),
    RestartableModelInterface(*this, /*read_only=*/false, _type + "_" + name()),
    _variable_names(isParamValid("filename")
                        ? getModelData<std::vector<VariableName>>("variables")
                        : declareModelData<std::vector<VariableName>>(
                              "variables", getParam<std::vector<VariableName>>("variables"))),
    _mapping_ready_to_use(declareModelData<std::map<VariableName, bool>>("mapping_ready_to_use"))
{
}

void
VariableMappingBase::checkIfReadyToUse(const VariableName & libmesh_dbg_var(vname)) const
{
  mooseAssert(_mapping_ready_to_use.find(vname) != _mapping_ready_to_use.end() &&
                  _mapping_ready_to_use[vname],
              "The mapping for variable " + vname + "is not ready to use!");
}
