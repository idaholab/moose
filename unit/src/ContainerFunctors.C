//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "Registry.h"
#include "MooseMesh.h"
#include "MooseUnitApp.h"
#include "AppFactory.h"
#include "Factory.h"
#include "InputParameters.h"
#include "MeshGeneratorMesh.h"
#include "MooseError.h"
#include "CastUniquePointer.h"
#include "GeneratedMeshGenerator.h"
#include "MooseFunctor.h"
#include "FaceInfo.h"
#include "MooseTypes.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "libmesh/elem.h"

#include <memory>

using namespace Moose;
using namespace Moose::FV;
using namespace MooseUtils;

TEST(ContainerFunctors, Test)
{
  const char * argv[2] = {"foo", "\0"};

  MultiMooseEnum coord_type_enum("XYZ RZ RSPHERICAL", "XYZ");

  constexpr auto nx = 2;
  auto app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
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
    params.set<unsigned int>("nx") = nx;
    params.set<unsigned int>("ny") = nx;
    params.set<MooseEnum>("dim") = "2";
    auto mesh_gen =
        factory->create<GeneratedMeshGenerator>("GeneratedMeshGenerator", "mesh_gen", params);
    lm_mesh = mesh_gen->generate();
    mesh->setMeshBase(std::move(lm_mesh));
  }

  mesh->prepare(nullptr);
  mesh->setCoordSystem({}, coord_type_enum);
  mooseAssert(mesh->getAxisymmetricRadialCoord() == 0,
              "This should be 0 because we haven't set anything.");
  const auto & all_fi = mesh->allFaceInfo();
  mesh->computeFaceInfoFaceCoords();
  std::vector<const FaceInfo *> faces(all_fi.size());
  for (const auto i : index_range(all_fi))
    faces[i] = &all_fi[i];

  PiecewiseByBlockLambdaFunctor<std::vector<Real>> vector_functor(
      "vector_functor",
      [](const auto &, const auto &) -> std::vector<Real> { return {1}; },
      {EXEC_ALWAYS},
      *mesh,
      mesh->meshSubdomains());
  PiecewiseByBlockLambdaFunctor<std::array<Real, 1>> array_functor(
      "array_functor",
      [](const auto &, const auto &) -> std::array<Real, 1> { return {1}; },
      {EXEC_ALWAYS},
      *mesh,
      mesh->meshSubdomains());

  std::array<LimiterType, 3> limiter_types = {
      {LimiterType::Upwind, LimiterType::CentralDifference, LimiterType::SOU}};
  for (const auto limiter_type : limiter_types)
    for (const auto * const face : faces)
    {
      const auto face_arg = Moose::FaceArg({face, limiter_type, true, false, nullptr});
      const auto current_time = Moose::currentState();
      EXPECT_TRUE(vector_functor(face_arg, current_time)[0] == 1.);
      EXPECT_TRUE(array_functor(face_arg, current_time)[0] == 1.);

      const auto vector_face_gradient = vector_functor.gradient(face_arg, current_time)[0];
      const auto array_face_gradient = array_functor.gradient(face_arg, current_time)[0];
      const auto vector_elem_gradient =
          vector_functor.gradient(face_arg.makeElem(), current_time)[0];
      const auto array_elem_gradient = array_functor.gradient(face_arg.makeElem(), current_time)[0];
      const auto vector_neighbor_gradient =
          face->neighborPtr() ? vector_functor.gradient(face_arg.makeNeighbor(), current_time)[0]
                              : VectorValue<Real>();
      const auto array_neighbor_gradient =
          face->neighborPtr() ? array_functor.gradient(face_arg.makeNeighbor(), current_time)[0]
                              : VectorValue<Real>();

      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      {
        EXPECT_TRUE(absoluteFuzzyEqual(vector_face_gradient(i), 0));
        EXPECT_TRUE(absoluteFuzzyEqual(array_face_gradient(i), 0));
        EXPECT_TRUE(absoluteFuzzyEqual(vector_elem_gradient(i), 0));
        EXPECT_TRUE(absoluteFuzzyEqual(array_elem_gradient(i), 0));
        EXPECT_TRUE(absoluteFuzzyEqual(vector_neighbor_gradient(i), 0));
        EXPECT_TRUE(absoluteFuzzyEqual(array_neighbor_gradient(i), 0));
      }
    }
}
