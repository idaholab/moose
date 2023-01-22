//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MathFVUtils.h"
#include "FaceInfo.h"
#include "ElemInfo.h"
#include "UpwindLimiter.h"
#include "AppFactory.h"
#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/parallel_object.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

using namespace libMesh;
using namespace Moose::FV;

TEST(LimitersTest, limitVector)
{
  UpwindLimiter<Real> limiter;
  VectorValue<Real> upwind(1, 1, 1);
  VectorValue<Real> downwind(0, 0, 0);
  TensorValue<Real> grad(0);

  const char * argv[2] = {"foo", "\0"};
  auto app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  ReplicatedMesh mesh(app->comm(), /*dim=*/2);
  MeshTools::Generation::build_square(mesh, 2, 2);
  ElemInfo ei(mesh.elem_ptr(0));

  dof_id_type counter = 0;
  for (const auto s : ei.elem()->side_index_range())
    if (ei.elem()->neighbor_ptr(s))
    {
      FaceInfo fi(&ei, s, counter++);
      ElemInfo ni(ei.elem()->neighbor_ptr(s));
      fi.computeInternalCoefficients(&ni);
      auto result = interpolate(limiter, upwind, downwind, &grad, fi, true);
      for (const auto d : make_range(unsigned(LIBMESH_DIM)))
        EXPECT_EQ(result(d), 1);
    }
}

TEST(LimitersTest, limiterType)
{
  EXPECT_TRUE(limiterType(InterpMethod::Average) == LimiterType::CentralDifference);
  EXPECT_TRUE(limiterType(InterpMethod::SkewCorrectedAverage) == LimiterType::CentralDifference);
  EXPECT_TRUE(limiterType(InterpMethod::Upwind) == LimiterType::Upwind);
  EXPECT_TRUE(limiterType(InterpMethod::VanLeer) == LimiterType::VanLeer);
  EXPECT_TRUE(limiterType(InterpMethod::MinMod) == LimiterType::MinMod);
  EXPECT_TRUE(limiterType(InterpMethod::SOU) == LimiterType::SOU);
  EXPECT_TRUE(limiterType(InterpMethod::QUICK) == LimiterType::QUICK);
}
