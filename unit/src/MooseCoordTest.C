//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseAppCoordTransform.h"
#include "MooseUtils.h"
#include "Units.h"
#include "MeshGeneratorMesh.h"
#include "GeneratedMeshGenerator.h"
#include "InputParameters.h"
#include "AppFactory.h"
#include "MooseEnum.h"
#include "libmesh/mesh_base.h"
#include "libmesh/point.h"

using namespace MooseUtils;

TEST(MooseCoordTest, testRotations)
{
  MooseAppCoordTransform transform{};
  const auto x = MooseAppCoordTransform::X;
  const auto y = MooseAppCoordTransform::Y;
  const auto z = MooseAppCoordTransform::Z;
  const Point xpt(1, 0, 0);
  const Point ypt(0, 1, 0);
  const Point zpt(0, 0, 1);
  const Point minus_xpt(-1, 0, 0);
  const Point minus_ypt(0, -1, 0);
  const Point minus_zpt(0, 0, -1);
  MultiAppCoordTransform multi_transform(transform);
  multi_transform.setDestinationCoordTransform(transform);

  auto error_checking = [&transform](const auto up_direction, const auto & error_string)
  {
    try
    {
      transform.setUpDirection(up_direction);
      FAIL();
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(error_string) != std::string::npos);
    }
  };

  auto compare_points = [](const Point & pt1, const Point & pt2)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      EXPECT_NEAR(pt1(i), pt2(i), TOLERANCE * TOLERANCE);
  };

  transform.setCoordinateSystem(Moose::COORD_RZ, y);
  transform.setUpDirection(x);
  compare_points(multi_transform(xpt), ypt);

  transform.setCoordinateSystem(Moose::COORD_RZ, y);
  transform.setUpDirection(y);
  compare_points(multi_transform(xpt), xpt);

  transform.setCoordinateSystem(Moose::COORD_RZ, y);
  transform.setUpDirection(z);
  compare_points(multi_transform(xpt), xpt);
  compare_points(multi_transform(ypt), minus_zpt);

  transform.setCoordinateSystem(Moose::COORD_RZ, x);
  error_checking(x, "Rotation yields negative radial values");
  compare_points(multi_transform(ypt), minus_xpt);

  transform.setCoordinateSystem(Moose::COORD_RZ, x);
  transform.setUpDirection(y);
  compare_points(multi_transform(xpt), xpt);

  transform.setCoordinateSystem(Moose::COORD_RZ, x);
  error_checking(z, "Rotation yields negative radial values");
  compare_points(multi_transform(ypt), minus_zpt);

  auto error_angles_checking =
      [&transform](const auto alpha, const auto beta, const auto gamma, const auto & error_string)
  {
    try
    {
      transform.setRotation(alpha, beta, gamma);
      FAIL();
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(error_string) != std::string::npos);
    }
  };

  transform.setCoordinateSystem(Moose::COORD_RZ, x);
  error_angles_checking(0, 0, 0, "Unsupported manual angle prescription");

  transform.setCoordinateSystem(Moose::COORD_RZ, x);
  transform.setRotation(0, 90, 0);
  {
    // test copy operations
    auto dup = transform;
    transform = dup;
  }
  compare_points(multi_transform(ypt), zpt);
  compare_points(multi_transform(xpt), xpt);
}

TEST(MooseCoordTest, testCoordCollapse)
{
  MooseAppCoordTransform single_app_xyz{};
  single_app_xyz.setCoordinateSystem(Moose::COORD_XYZ);
  MooseAppCoordTransform single_app_rz{};
  single_app_rz.setCoordinateSystem(Moose::COORD_RZ, MooseAppCoordTransform::Y);
  MooseAppCoordTransform single_app_rsph{};
  single_app_rsph.setCoordinateSystem(Moose::COORD_RSPHERICAL);
  MultiAppCoordTransform xyz(single_app_xyz);
  MultiAppCoordTransform rz(single_app_rz);
  MultiAppCoordTransform rsph(single_app_rsph);

  const std::string return_mapping_error_message(
      "Coordinate collapsing occurred in going to the reference space. There is no unique return "
      "mapping");

  const Real sqrt2 = std::sqrt(2.);
  const Real sqrt3 = std::sqrt(3.);
  const Point xyz_pt(1, 1, 1);
  const Point rz_pt(1, 1, 0);
  {
    xyz.setDestinationCoordTransform(single_app_rz);
    const auto pt = xyz(xyz_pt);
    EXPECT_TRUE(absoluteFuzzyEqual(pt(0), sqrt2));
    EXPECT_TRUE(absoluteFuzzyEqual(pt(1), 1.));
    EXPECT_TRUE(absoluteFuzzyEqual(pt(2), 0.));
    EXPECT_TRUE(xyz.hasNonTranslationTransformation());
    try
    {
      xyz.mapBack(pt);
      FAIL();
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(return_mapping_error_message) != std::string::npos);
    }
  }
  {
    xyz.setDestinationCoordTransform(single_app_rsph);
    const auto pt = xyz(xyz_pt);
    EXPECT_TRUE(absoluteFuzzyEqual(pt(0), sqrt3));
    EXPECT_TRUE(absoluteFuzzyEqual(pt(1), 0.));
    EXPECT_TRUE(absoluteFuzzyEqual(pt(2), 0.));
    EXPECT_TRUE(xyz.hasNonTranslationTransformation());
    try
    {
      xyz.mapBack(pt);
      FAIL();
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(return_mapping_error_message) != std::string::npos);
    }
  }
  {
    rz.setDestinationCoordTransform(single_app_rsph);
    const auto pt = rz(rz_pt);
    EXPECT_TRUE(absoluteFuzzyEqual(pt(0), sqrt2));
    EXPECT_TRUE(absoluteFuzzyEqual(pt(1), 0.));
    EXPECT_TRUE(absoluteFuzzyEqual(pt(2), 0.));
    EXPECT_TRUE(rz.hasNonTranslationTransformation());
    try
    {
      rz.mapBack(pt);
      FAIL();
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(return_mapping_error_message) != std::string::npos);
    }
  }
}

TEST(MooseCoordTest, testLengthUnit)
{
  const char * argv[2] = {"foo", "\0"};

  const auto nx = 2;
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

  mesh->prepare();

  EXPECT_TRUE(mesh->lengthUnit() == MooseUnits("1*m"));
}
