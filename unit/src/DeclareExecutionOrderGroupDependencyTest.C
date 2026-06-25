//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeclareExecutionOrderGroupDependencyTest.h"
#include "GeneralUserObject.h"
#include "SideUserObject.h"

TEST_F(DeclareExecutionOrderGroupDependencyTest, Side0Side0)
{
  addIndependentUserObject("objA", false, 0);
  ASSERT_THROW(addDependentUserObject("objB", false, "objA", false, 0), std::runtime_error);
}

TEST_F(DeclareExecutionOrderGroupDependencyTest, Side0Side1)
{
  addIndependentUserObject("objA", false, 0);
  ASSERT_NO_THROW(addDependentUserObject("objB", false, "objA", false, 1));
}

TEST_F(DeclareExecutionOrderGroupDependencyTest, Side0General0)
{
  addIndependentUserObject("objA", false, 0);
  ASSERT_NO_THROW(addDependentUserObject("objB", true, "objA", false, 0));
}

TEST_F(DeclareExecutionOrderGroupDependencyTest, Side1General0)
{
  addIndependentUserObject("objA", false, 1);
  ASSERT_THROW(addDependentUserObject("objB", true, "objA", false, 0), std::runtime_error);
}

TEST_F(DeclareExecutionOrderGroupDependencyTest, General0Side0)
{
  addIndependentUserObject("objA", true, 0);
  ASSERT_THROW(addDependentUserObject("objB", false, "objA", true, 0), std::runtime_error);
}

TEST_F(DeclareExecutionOrderGroupDependencyTest, General0Side1)
{
  addIndependentUserObject("objA", true, 0);
  ASSERT_NO_THROW(addDependentUserObject("objB", false, "objA", true, 1));
}

void
DeclareExecutionOrderGroupDependencyTest::addDependentUserObject(std::string uo_name,
                                                                 bool is_general_uo,
                                                                 std::string depends_on_uo_name,
                                                                 bool depends_on_is_general_uo,
                                                                 int execution_order_group)
{
  std::string class_name;
  if (is_general_uo)
    class_name = "ExecutionOrderGroupDependencyGeneralUserObject";
  else
    class_name = "ExecutionOrderGroupDependencySideUserObject";
  InputParameters params = _factory.getValidParams(class_name);
  if (!is_general_uo)
    params.set<std::vector<BoundaryName>>("boundary") = {};
  params.set<int>("execution_order_group") = execution_order_group;
  params.set<UserObjectName>("depends_on") = depends_on_uo_name;
  params.set<bool>("depends_on_is_general_uo") = depends_on_is_general_uo;
  _fe_problem->addUserObject(class_name, uo_name, params);
}

void
DeclareExecutionOrderGroupDependencyTest::addIndependentUserObject(std::string uo_name,
                                                                   bool is_general_uo,
                                                                   int execution_order_group)
{
  std::string class_name;
  if (is_general_uo)
    class_name = "ExecutionOrderGroupDependencyGeneralUserObject";
  else
    class_name = "ExecutionOrderGroupDependencySideUserObject";
  InputParameters params = _factory.getValidParams(class_name);
  if (!is_general_uo)
    params.set<std::vector<BoundaryName>>("boundary") = {};
  params.set<int>("execution_order_group") = execution_order_group;
  _fe_problem->addUserObject(class_name, uo_name, params);
}

typedef ExecutionOrderGroupDependencyUserObject<GeneralUserObject>
    ExecutionOrderGroupDependencyGeneralUserObject;
typedef ExecutionOrderGroupDependencyUserObject<SideUserObject>
    ExecutionOrderGroupDependencySideUserObject;

registerMooseObject("MooseUnitApp", ExecutionOrderGroupDependencyGeneralUserObject);
registerMooseObject("MooseUnitApp", ExecutionOrderGroupDependencySideUserObject);

template <typename BaseType>
InputParameters
ExecutionOrderGroupDependencyUserObject<BaseType>::validParams()
{
  InputParameters params = BaseType::validParams();
  params.addParam<UserObjectName>("depends_on", "User object that this one depends on");
  params.addParam<bool>("depends_on_is_general_uo",
                        "Is the type of 'depends_on' a general UO? Else, side UO");
  return params;
}

template <typename BaseType>
ExecutionOrderGroupDependencyUserObject<BaseType>::ExecutionOrderGroupDependencyUserObject(
    const InputParameters & params)
  : BaseType(params)
{
  if (BaseType::isParamValid("depends_on"))
  {
    if (this->template getParam<bool>("depends_on_is_general_uo"))
    {
      const auto & depends_on =
          this->template getUserObject<ExecutionOrderGroupDependencyGeneralUserObject>(
              "depends_on");
      BaseType::declareExecutionOrderGroupDependency(depends_on);
    }
    else
    {
      const auto & depends_on =
          this->template getUserObject<ExecutionOrderGroupDependencySideUserObject>("depends_on");
      BaseType::declareExecutionOrderGroupDependency(depends_on);
    }
  }
}
