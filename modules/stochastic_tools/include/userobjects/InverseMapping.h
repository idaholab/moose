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
#include "UserObject.h"
#include "MappingInterface.h"
#include "SurrogateModelInterface.h"

class InverseMapping : public UserObject
{
public:
  static InputParameters validParams();

  InverseMapping(const InputParameters & params);

  void execute() override;
  void initialize() override {}
  void finalize() override {}

  void threadJoin(const UserObject & uo) override{};

protected:
  MooseVariableBase * _variable_to_fill;
  MappingBase * _mapping;
  SurrogateModel * _surrogate;

  const std::vector<Real> & _input_parameters;
};
