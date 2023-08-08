//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementReporter.h"

class ElementIDTest : public ElementReporter
{
public:
  static InputParameters validParams();

  ElementIDTest(const InputParameters & params);

  void initialize() override {}
  void execute() override {}
  void finalize() override {}
  void threadJoin(const UserObject &) override {}

protected:
  const ExtraElementIDName & _id_name1;
  const ExtraElementIDName & _id_name2;
  std::unordered_map<dof_id_type, std::set<dof_id_type>> & _mapping;
};
