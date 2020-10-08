#include "gtest/gtest.h"

#include "BuildMeshTest.h"
#include "Registry.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseUnitApp.h"
#include "AppFactory.h"
#include "Factory.h"
#include "InputParameters.h"
#include "MeshGeneratorMesh.h"
#include "MooseError.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/distributed_mesh.h"

#include <memory>

registerMooseObject("MooseUnitApp", BuildMeshBaseTypesGenerator);

std::unique_ptr<MeshBase>
BuildMeshBaseTypesGenerator::generate()
{
  auto mesh = _mesh->buildMeshBaseObject();

  auto rep_mesh = buildReplicatedMesh();
  auto dist_mesh = buildDistributedMesh();

  const Point p_rep(0), p_dist(0);

  rep_mesh->add_point(p_rep);
  dist_mesh->add_point(p_dist);

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
BuildMeshTest::SetUp()
{
  const char * argv[2] = {"foo", "\0"};
  _app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();
  std::string mesh_type = "MeshGeneratorMesh";
  std::string mesh_gen_type = "BuildMeshBaseTypesGenerator";

  {
    InputParameters params = _factory->getValidParams(mesh_type);
    _moose_mesh = _factory->create<MeshGeneratorMesh>(mesh_type, "moose_mesh", params);
  }

  _app->actionWarehouse().mesh() = _moose_mesh;

  {
    InputParameters params = _factory->getValidParams(mesh_gen_type);
    _mesh_gen = _factory->create<BuildMeshBaseTypesGenerator>(mesh_gen_type, "mesh_gen", params);
  }
}

TEST_F(BuildMeshTest, theTest) { _mesh_gen->generate(); }
