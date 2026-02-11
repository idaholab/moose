//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "hit/hit.h"

#include "ParameterRegistry.h"
#include "MooseUnitUtils.h"

TEST(ParameterRegistryTest, add)
{
  Moose::ParameterRegistry registry;
  ASSERT_TRUE(registry._registry.empty());
  registry.add<double>([](double &, const hit::Field &) {});
  ASSERT_EQ(registry._registry.size(), 1);
  ASSERT_TRUE(registry._registry.find(std::type_index(typeid(double))) != registry._registry.end());
}

TEST(ParameterRegistryTest, addExists)
{
  Moose::ParameterRegistry registry;
  const auto add = [&registry]() { registry.add<double>([](double &, const hit::Field &) {}); };
  add();
  EXPECT_MOOSEERROR_MSG(add(),
                        "ParameterRegistry: Parameter with type 'double' is already registered");
}

TEST(ParameterRegistryTest, set)
{
  Moose::ParameterRegistry registry;
  registry.add<double>([](double & value, const hit::Field & field)
                       { value = field.param<double>(); });

  libMesh::Parameters params;
  params.set<double>("value") = 0;
  const auto & value = params.get<double>("value");
  auto & param = *params.begin()->second.get();
  ASSERT_EQ(value, 0);

  const std::unique_ptr<const hit::Node> root(hit::parse("file", "value = 1"));
  const auto node = root->find("value");
  ASSERT_TRUE(node);
  const auto field = dynamic_cast<const hit::Field *>(node);
  ASSERT_TRUE(field);
  registry.set(param, *field);
  ASSERT_EQ(value, 1);
}

TEST(ParameterRegistryTest, setNotRegistered)
{
  const Moose::ParameterRegistry registry;

  libMesh::Parameters params;
  params.set<double>("value");
  auto & param = *params.begin()->second.get();

  const std::unique_ptr<const hit::Node> root(hit::parse("file", "value = 1"));
  const auto field = dynamic_cast<const hit::Field *>(root->find("value"));
  EXPECT_MOOSEERROR_MSG(registry.set(param, *field),
                        "ParameterRegistry::set(): Parameter type 'double' is not registered");
}

TEST(ParameterRegistryTest, setCatchMooseError)
{
  Moose::ParameterRegistry registry;
  registry.add<int>([](int &, const hit::Field &) { ::mooseError("foo"); });

  libMesh::Parameters params;
  params.set<int>("value");
  auto & param = *params.begin()->second.get();

  const std::unique_ptr<const hit::Node> root(hit::parse("file", "value = 1"));
  const auto field = dynamic_cast<const hit::Field *>(root->find("value"));

  EXPECT_MOOSEERROR_MSG(registry.set(param, *field), "foo");
}
