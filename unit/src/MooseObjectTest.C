//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MooseMain.h"
#include "MeshGeneratorMesh.h"

template <class T, bool shared>
void
getSharedTest()
{
  // create an app to obtain a factory
  std::shared_ptr<MooseApp> app = Moose::createMooseApp("MooseUnitApp", 0, nullptr);
  auto * factory = &app->getFactory();

  // build a MooseObject
  std::string type = "MeshGeneratorMesh";
  InputParameters params = factory->getValidParams(type);

  if constexpr (shared)
  {
    auto mesh = factory->create<T>(type, "mesh", params);

    // check usage
    EXPECT_EQ(mesh.use_count(), 1);
    {
      auto mesh2 = mesh->getSharedPtr();
      EXPECT_EQ(mesh.use_count(), 2);
    }
    EXPECT_EQ(mesh.use_count(), 1);
  }
  else
  {
    auto mesh = factory->createUnique<T>(type, "mesh", params);
    EXPECT_THROW({ auto mesh2 = mesh->getSharedPtr(); }, std::exception);
  }
}

TEST(MooseObjectTest, getSharedPtr_base) { getSharedTest<MooseObject, true>(); }
TEST(MooseObjectTest, getSharedPtr_derived) { getSharedTest<MeshGeneratorMesh, true>(); }
TEST(MooseObjectTest, getSharedPtr_error) { getSharedTest<MeshGeneratorMesh, false>(); }
