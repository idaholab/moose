//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"

class DeclareExecutionOrderGroupDependencyTest : public MooseObjectUnitTest
{
public:
  DeclareExecutionOrderGroupDependencyTest() : MooseObjectUnitTest("MooseUnitApp") {}

  void addDependentUserObject(std::string uo_name,
                              bool is_general_uo,
                              std::string depends_on_uo_name,
                              bool depends_on_is_general_uo,
                              int execution_order_group);
  void addIndependentUserObject(std::string uo_name, bool is_general_uo, int execution_order_group);
};

template <typename BaseType>
class ExecutionOrderGroupDependencyUserObject : public BaseType
{
public:
  static InputParameters validParams();

  ExecutionOrderGroupDependencyUserObject(const InputParameters & params);

  virtual void initialize() override {};
  virtual void execute() override {};
  virtual void finalize() override {};
  virtual void threadJoin(const UserObject &) override {};
};
