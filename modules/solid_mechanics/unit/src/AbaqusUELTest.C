//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "AbaqusUELMesh.h"
#include "MooseMain.h"
#include "DataFileUtils.h"
#include "AbaqusInputParser.h"
namespace
{

std::shared_ptr<AbaqusUELMesh>
loadUELMesh(const std::string & file)
{
  const char * argv[1] = {"\0"};
  std::shared_ptr<MooseApp> app = Moose::createMooseApp("SolidMechanicsApp", 1, (char **)argv);
  auto * factory = &app->getFactory();

  std::string mesh_type = "AbaqusUELMesh";
  InputParameters params = factory->getValidParams(mesh_type);
  params.set<FileName>("file") = Moose::DataFileUtils::getPath("solid_mechanics:unit/" + file).path;
  auto mesh = factory->create<AbaqusUELMesh>(mesh_type, "uel_mesh", params);
  app->actionWarehouse().mesh() = mesh;
  mesh->init();
  return mesh;
}

void
exceptionTest(const std::string & file, const std::string & expect_msg)
{
  try
  {
    auto mesh = loadUELMesh(file);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find(expect_msg) != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

}

TEST(AbaqusUELTest, EmptyMesh)
{
  auto mesh = loadUELMesh("empty.inp");
  EXPECT_EQ(mesh->getElements().size(), 0u);
}

// Assembly instantiation currently raises a runtime error in Instance::Instance.
// Disable this test until instantiation is completed.
TEST(AbaqusUELTest, Square)
{
  auto mesh = loadUELMesh("square.inp");
  const auto & elements = mesh->getElements();
  ASSERT_EQ(elements.size(), 1u);

  static const Real gold_nodes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  const auto & nodes = elements[0]._nodes;
  ASSERT_EQ(nodes.size(), 9u);

  static const Real gold_coords[][2] = {{-1., -1.},
                                        {1., -1.},
                                        {1., 1.},
                                        {-1., 1.},
                                        {0., -1.},
                                        {1., 0.},
                                        {0., 1.},
                                        {-1., 0.},
                                        {0., 0.}};

  static const subdomain_id_type gold_block[] = {131, 131, 131, 131, 3, 3, 3, 3, 8};

  for (const auto i : make_range(9))
  {
    // check node IDs
    EXPECT_EQ(nodes[i], gold_nodes[i]);

    // check node coordinates
    const auto & p = mesh->nodeRef(nodes[i]);
    EXPECT_EQ(p(0), gold_coords[i][0]);
    EXPECT_EQ(p(1), gold_coords[i][1]);

    // check blocks
    const auto * e = mesh->elemPtr(nodes[i]);
    EXPECT_EQ(e->subdomain_id(), gold_block[i]);
  }
}

TEST(AbaqusUELTest, ParseErrors)
{
  // Current implementation treats Part-only files as unsupported in flat mode
  const std::string expect = "Unsupported block part";
  exceptionTest("error_coord.inp", expect);
  exceptionTest("error_unknown_uel.inp", expect);
  exceptionTest("error_node_count.inp", expect);
  exceptionTest("error_node_num.inp", expect);
}
