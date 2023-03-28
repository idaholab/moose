//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralUserObject.h"
#include "MappingInterface.h"
#include "SurrogateModelInterface.h"
#include "SurrogateModel.h"

class InverseMapping : public GeneralUserObject,
                       public MappingInterface,
                       public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  InverseMapping(const InputParameters & params);

  void execute() override;
  void initialize() override {}
  void finalize() override {}

  void initialSetup() override;

  void threadJoin(const UserObject & /*uo*/) override{};

protected:
  const std::vector<VariableName> & _var_names_to_fill;
  const std::vector<VariableName> & _var_names_to_reconstruct;
  const std::vector<UserObjectName> & _surrogate_model_names;

  std::vector<MooseVariableFieldBase *> _variable_to_fill;
  std::vector<const MooseVariableFieldBase *> _variable_to_reconstruct;
  std::vector<std::unordered_map<dof_id_type, dof_id_type>> _variable_dof_to_row;
  std::vector<bool> _is_nodal;
  MappingBase * _mapping;
  std::vector<SurrogateModel *> _surrogate_models;

  const std::vector<Real> & _input_parameters;
};
