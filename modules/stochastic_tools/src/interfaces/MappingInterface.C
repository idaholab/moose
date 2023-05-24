//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MappingInterface.h"
#include "MooseTypes.h"

InputParameters
MappingInterface::validParams()
{
  return emptyInputParameters();
}

MappingInterface::MappingInterface(const MooseObject * moose_object)
  : _smi_params(moose_object->parameters()),
    _smi_feproblem(*_smi_params.get<FEProblemBase *>("_fe_problem_base"))
{
}

VariableMappingBase &
MappingInterface::getMappingByName(const UserObjectName & name) const
{
  std::vector<VariableMappingBase *> models;
  _smi_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("VariableMappingBase")
      .queryInto(models);
  if (models.empty())
    mooseError("Unable to find a Mapping object with the name '" + name + "'");
  return *(models[0]);
}

VariableMappingBase &
MappingInterface::getMapping(const std::string & name) const
{
  return getMappingByName(_smi_params.get<UserObjectName>(name));
}
