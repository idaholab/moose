//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "gtest_include.h"

#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMain.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

class AssemblyReinitDualTest : public ::testing::Test
{
public:
  AssemblyReinitDualTest()
    : _app(Moose::createMooseApp("MooseUnitApp", 0, nullptr)), _factory(_app->getFactory())
  {
    InputParameters mesh_params = _factory.getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "2";
    mesh_params.set<MooseEnum>("elem_type") = "QUAD9";
    _mesh = _factory.createUnique<MooseMesh>("GeneratedMesh", "moose_mesh", mesh_params);
    _mesh->setMeshBase(_mesh->buildMeshBaseObject());
    _mesh->buildMesh();
    _mesh->prepare();

    InputParameters problem_params = _factory.getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>(MooseBase::name_param) = "problem";
    _problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);
    _problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);
    _app->actionWarehouse().problemBase() = _problem;
  }

protected:
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<FEProblem> _problem;
};

TEST_F(AssemblyReinitDualTest, ReinitOnlyRequestedTypes)
{
  auto & assembly = _problem->assembly(0, 0);
  const libMesh::FEType first_lagrange(libMesh::FIRST, libMesh::LAGRANGE);
  const auto & dual_phi = assembly.feDualPhiLower<Real>(first_lagrange);

  const auto * elem = *_mesh->getMesh().active_local_element_ptr_range().begin();
  const auto side = elem->build_side_ptr(0);
  const std::vector<Point> points = {Point(-1), Point(1)};
  const std::vector<Real> weights(2, 1);

  EXPECT_NO_THROW(assembly.reinitDual(side.get(), points, weights));
  assembly.reinitLowerDElem(side.get(), &points, &weights);

  ASSERT_EQ(dual_phi.size(), 2);
  for (const auto qp : make_range(points.size()))
    EXPECT_DOUBLE_EQ(dual_phi[0][qp] + dual_phi[1][qp], 1);
}
