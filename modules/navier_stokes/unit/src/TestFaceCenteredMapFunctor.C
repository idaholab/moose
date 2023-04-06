//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MeshGeneratorMesh.h"
#include "GeneratedMeshGenerator.h"
#include "FaceCenteredMapFunctor.h"
#include "AppFactory.h"
#include "libmesh/quadrature_gauss.h"

using namespace libMesh;
using namespace Moose;
using namespace FV;

template <typename T, typename Map>
class FictionalFaceCenteredMapFunctor : public FaceCenteredMapFunctor<T, Map>
{
public:
  FictionalFaceCenteredMapFunctor(const MooseMesh & mesh,
                                  const std::set<SubdomainID> & sub_ids,
                                  const std::string & name)
    : FaceCenteredMapFunctor<T, Map>(mesh, sub_ids, name)
  {
  }

  bool hasBlocks(SubdomainID) const override { return true; }
};

TEST(FaceCenteredMapFunctorTest, testArgs)
{
  const char * argv[2] = {"foo", "\0"};

  // First we create a simple mesh
  auto app = AppFactory::createAppShared("NavierStokesUnitApp", 1, (char **)argv);
  auto * factory = &app->getFactory();
  std::string mesh_type = "MeshGeneratorMesh";

  std::shared_ptr<MeshGeneratorMesh> mesh;
  {
    InputParameters params = factory->getValidParams(mesh_type);
    mesh = factory->create<MeshGeneratorMesh>(mesh_type, "moose_mesh", params);
  }

  app->actionWarehouse().mesh() = mesh;

  {
    std::unique_ptr<MeshBase> lm_mesh;
    InputParameters params = factory->getValidParams("GeneratedMeshGenerator");
    params.set<unsigned int>("nx") = 2;
    params.set<unsigned int>("ny") = 2;
    params.set<MooseEnum>("dim") = "2";
    auto mesh_gen =
        factory->create<GeneratedMeshGenerator>("GeneratedMeshGenerator", "mesh_gen", params);
    lm_mesh = mesh_gen->generate();
    mesh->setMeshBase(std::move(lm_mesh));
  }

  mesh->prepare();
  MultiMooseEnum coord_type_enum("XYZ RZ RSPHERICAL", "XYZ");
  mesh->setCoordSystem({}, coord_type_enum);
  const auto & all_fi = mesh->allFaceInfo();
  mesh->computeFaceInfoFaceCoords();

  // We create a face-centered functor
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> u(*mesh,
                                                                                              "u");

  // We fill up the functor with known values
  for (auto & fi : all_fi)
  {
    const auto & face_center = fi.faceCentroid();
    u[fi.id()] = RealVectorValue(
        -sin(face_center(0)) * cos(face_center(1)), cos(face_center(0)) * sin(face_center(1)), 0);
  }

  // We check if the functor has the right face values
  for (auto & fi : all_fi)
  {
    const auto & face_center = fi.faceCentroid();
    const auto face_arg =
        Moose::FaceArg{&fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};

    const auto result = u(face_arg, Moose::currentState());

    EXPECT_NEAR(result(0), -sin(face_center(0)) * cos(face_center(1)), 1e-14);
    EXPECT_NEAR(result(1), cos(face_center(0)) * sin(face_center(1)), 1e-14);
    EXPECT_EQ(result(2), 0);
  }

  // Next, we test for the error messages of not-implemented methods
  // This is a lambda for testing errors when requesting a gradient evaluation
  auto test_gradient = [&u](const auto & arg)
  {
    try
    {
      u.gradient(arg, Moose::currentState());
      EXPECT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      EXPECT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
    }
  };

  // This is a lambda for testing errors when requesting a regular evaluation with an unsupported
  // argument
  auto test_evaluate = [&u](const auto & arg)
  {
    try
    {
      u(arg, Moose::currentState());
      EXPECT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      EXPECT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
    }
  };

  // Arguments for the simple error checks, we use the first face and the corresponding
  // owner element
  QGauss qrule(1, CONSTANT);
  const auto face_arg =
      Moose::FaceArg{&all_fi[0], Moose::FV::LimiterType::CentralDifference, true, false, nullptr};
  const auto elem_arg = ElemArg{all_fi[0].elemPtr(), false};
  const auto elem_qp_arg = std::make_tuple(all_fi[0].elemPtr(), 0, &qrule);
  const auto elem_side_qp_arg = std::make_tuple(all_fi[0].elemPtr(), 0, 0, &qrule);
  const auto elem_point_arg = ElemPointArg({all_fi[0].elemPtr(), Point(0), false});

  test_gradient(elem_arg);
  test_gradient(face_arg);

  test_evaluate(elem_qp_arg);
  test_evaluate(elem_side_qp_arg);
  test_evaluate(elem_point_arg);

  // Lastly, we check for errors when encountering faces with incorrect subdomains
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      unrestricted_error_test(*mesh, "not_restricted");
  try
  {
    unrestricted_error_test(
        FaceArg{&all_fi[2], LimiterType::CentralDifference, true, false, nullptr},
        Moose::currentState());
    EXPECT_TRUE(false);
  }
  catch (std::runtime_error & e)
  {
    EXPECT_TRUE(std::string(e.what()).find("not_restricted") != std::string::npos);
    EXPECT_TRUE(std::string(e.what()).find("Make sure to fill") != std::string::npos);
  }

  FictionalFaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      restricted_error_test(*mesh, {1}, "is_restricted");
  try
  {
    restricted_error_test(FaceArg{&all_fi[2], LimiterType::CentralDifference, true, false, nullptr},
                          Moose::currentState());
    EXPECT_TRUE(false);
  }
  catch (std::runtime_error & e)
  {
    EXPECT_TRUE(std::string(e.what()).find("is_restricted") != std::string::npos);
    EXPECT_TRUE(std::string(e.what()).find("0") != std::string::npos);
    EXPECT_TRUE(
        std::string(e.what()).find(
            "that subdomain id is not one of the subdomain ids the functor is restricted to") !=
        std::string::npos);
  }
}
