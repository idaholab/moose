//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MooseFunctor.h"
#include "FaceInfo.h"
#include "ElemInfo.h"
#include "VectorComponentFunctor.h"
#include "GreenGaussGradient.h"
#include "MeshGeneratorMesh.h"
#include "GeneratedMeshGenerator.h"
#include "AppFactory.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "ADWrapperFunctor.h"
#include "RawValueFunctor.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/type_tensor.h"

using namespace libMesh;
using namespace Moose;
using namespace FV;

template <typename T>
class TestFunctor : public FunctorBase<T>
{
public:
  using typename FunctorBase<T>::ValueType;

  TestFunctor() : FunctorBase<T>("test"){};

  bool hasBlocks(SubdomainID) const override { return true; }

private:
  ValueType evaluate(const ElemArg &, const StateArg &) const override final { return 0; }
  ValueType evaluate(const FaceArg &, const StateArg &) const override final { return 0; }
  ValueType evaluate(const ElemQpArg &, const StateArg &) const override final { return 0; }
  ValueType evaluate(const ElemSideQpArg &, const StateArg &) const override final { return 0; }
  ValueType evaluate(const ElemPointArg &, const StateArg &) const override final { return 0; }
};

template <typename T>
class WithGradientTestFunctor : public TestFunctor<T>
{
public:
  using typename TestFunctor<T>::GradientType;

  WithGradientTestFunctor(const MooseMesh & mesh) : _mesh(mesh) {}

  bool
  isExtrapolatedBoundaryFace(const FaceInfo & fi, const Elem *, const StateArg &) const override
  {
    return !fi.neighborPtr();
  }

private:
  using TestFunctor<T>::evaluateGradient;
  GradientType evaluateGradient(const ElemArg & elem_arg,
                                const StateArg & time) const override final
  {
    return greenGaussGradient(elem_arg, time, *this, true, _mesh);
  }

  GradientType evaluateGradient(const FaceArg & face, const StateArg & time) const override final
  {
    return greenGaussGradient(face, time, *this, true, _mesh);
  }

  const MooseMesh & _mesh;
};

template <typename T>
class BypassFaceError : public PiecewiseByBlockLambdaFunctor<T>
{
public:
  template <typename PolymorphicLambda>
  BypassFaceError(const std::string & name,
                  PolymorphicLambda my_lammy,
                  const std::set<ExecFlagType> & clearance_schedule,
                  const MooseMesh & mesh,
                  const std::set<SubdomainID> & block_ids)
    : PiecewiseByBlockLambdaFunctor<T>(name, my_lammy, clearance_schedule, mesh, block_ids)
  {
  }

  bool hasFaceSide(const FaceInfo &, bool) const override { return true; }
};

TEST(MooseFunctorTest, testArgs)
{
  TestFunctor<Real> test;
  Node node0(0);
  node0.set_id(0);
  Node node1(1);
  node1.set_id(1);
  Node node2(2);
  node2.set_id(2);
  auto elem = Elem::build(EDGE2);
  elem->set_node(0) = &node0;
  elem->set_node(1) = &node1;
  auto neighbor = Elem::build(EDGE2);
  neighbor->set_node(0) = &node1;
  neighbor->set_node(1) = &node2;
  elem->set_neighbor(1, neighbor.get());

  ElemInfo ei(elem.get());
  ElemInfo ni(neighbor.get());
  FaceInfo fi(&ei, 1, 0);
  fi.computeInternalCoefficients(&ni);

  QGauss qrule(1, CONSTANT);

  auto elem_arg = ElemArg{elem.get(), false};
  auto face = FaceArg({&fi, LimiterType::CentralDifference, true, false, nullptr});
  auto elem_qp = std::make_tuple(elem.get(), 0, &qrule);
  auto elem_side_qp = std::make_tuple(elem.get(), 0, 0, &qrule);
  auto elem_point = ElemPointArg({elem.get(), Point(0), false});
  const auto current_time = Moose::currentState();

  // Test not-implemented errors
  {
    auto test_dot = [&test, &current_time](const auto & arg)
    {
      try
      {
        test.dot(arg, current_time);
        EXPECT_TRUE(false);
      }
      catch (std::runtime_error & e)
      {
        EXPECT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
      }
    };

    test_dot(elem_arg);
    test_dot(face);
    test_dot(elem_qp);
    test_dot(elem_side_qp);
    test_dot(elem_point);

    auto test_gradient = [&test, &current_time](const auto & arg)
    {
      try
      {
        test.gradient(arg, current_time);
        EXPECT_TRUE(false);
      }
      catch (std::runtime_error & e)
      {
        EXPECT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
      }
    };

    test_gradient(elem_arg);
    test_gradient(face);
    test_gradient(elem_qp);
    test_gradient(elem_side_qp);
    test_gradient(elem_point);
  }

  auto zero_gradient_test = [&current_time](const auto & functor, const auto & arg)
  {
    const auto result = functor.gradient(arg, current_time);
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
      EXPECT_EQ(result(i), 0);
  };

  // Test ConstantFunctor
  {
    ConstantFunctor<Real> cf(2);
    EXPECT_EQ(cf(elem_arg, current_time), 2);
    EXPECT_EQ(cf(face, current_time), 2);
    EXPECT_EQ(cf(elem_qp, current_time), 2);
    EXPECT_EQ(cf(elem_side_qp, current_time), 2);
    EXPECT_EQ(cf(elem_point, current_time), 2);

    zero_gradient_test(cf, elem_arg);
    zero_gradient_test(cf, face);
    zero_gradient_test(cf, elem_qp);
    zero_gradient_test(cf, elem_side_qp);
    zero_gradient_test(cf, elem_point);

    EXPECT_EQ(cf.dot(elem_arg, current_time), 0);
    EXPECT_EQ(cf.dot(face, current_time), 0);
    EXPECT_EQ(cf.dot(elem_qp, current_time), 0);
    EXPECT_EQ(cf.dot(elem_side_qp, current_time), 0);
    EXPECT_EQ(cf.dot(elem_point, current_time), 0);

    // Test AD up-type
    ADWrapperFunctor<ADReal> ad_cf(cf);
    EXPECT_EQ(cf(elem_arg, current_time), MetaPhysicL::raw_value(ad_cf(elem_arg, current_time)));
    EXPECT_EQ(cf(face, current_time), MetaPhysicL::raw_value(ad_cf(face, current_time)));
    EXPECT_EQ(cf(elem_point, current_time),
              MetaPhysicL::raw_value(ad_cf(elem_point, current_time)));
    EXPECT_EQ(cf(elem_qp, current_time), MetaPhysicL::raw_value(ad_cf(elem_qp, current_time)));
    EXPECT_EQ(cf(elem_side_qp, current_time),
              MetaPhysicL::raw_value(ad_cf(elem_side_qp, current_time)));
    EXPECT_EQ(cf.gradient(elem_arg, current_time)(0),
              MetaPhysicL::raw_value(ad_cf.gradient(elem_arg, current_time)(0)));
    EXPECT_EQ(cf.gradient(face, current_time)(0),
              MetaPhysicL::raw_value(ad_cf.gradient(face, current_time)(0)));
    EXPECT_EQ(cf.gradient(elem_point, current_time)(0),
              MetaPhysicL::raw_value(ad_cf.gradient(elem_point, current_time)(0)));
    EXPECT_EQ(cf.gradient(elem_qp, current_time)(0),
              MetaPhysicL::raw_value(ad_cf.gradient(elem_qp, current_time)(0)));
    EXPECT_EQ(cf.gradient(elem_side_qp, current_time)(0),
              MetaPhysicL::raw_value(ad_cf.gradient(elem_side_qp, current_time)(0)));
    EXPECT_EQ(cf.dot(elem_arg, current_time),
              MetaPhysicL::raw_value(ad_cf.dot(elem_arg, current_time)));
    EXPECT_EQ(cf.dot(face, current_time), MetaPhysicL::raw_value(ad_cf.dot(face, current_time)));
    EXPECT_EQ(cf.dot(elem_point, current_time),
              MetaPhysicL::raw_value(ad_cf.dot(elem_point, current_time)));
    EXPECT_EQ(cf.dot(elem_qp, current_time),
              MetaPhysicL::raw_value(ad_cf.dot(elem_qp, current_time)));
    EXPECT_EQ(cf.dot(elem_side_qp, current_time),
              MetaPhysicL::raw_value(ad_cf.dot(elem_side_qp, current_time)));

    // Test AD down-type
    RawValueFunctor<Real> raw_ad_cf(ad_cf);
    EXPECT_EQ(cf(elem_arg, current_time), raw_ad_cf(elem_arg, current_time));
    EXPECT_EQ(cf(face, current_time), raw_ad_cf(face, current_time));
    EXPECT_EQ(cf(elem_point, current_time), raw_ad_cf(elem_point, current_time));
    EXPECT_EQ(cf(elem_qp, current_time), raw_ad_cf(elem_qp, current_time));
    EXPECT_EQ(cf(elem_side_qp, current_time), raw_ad_cf(elem_side_qp, current_time));
    EXPECT_EQ(cf.gradient(elem_arg, current_time)(0),
              raw_ad_cf.gradient(elem_arg, current_time)(0));
    EXPECT_EQ(cf.gradient(face, current_time)(0), raw_ad_cf.gradient(face, current_time)(0));
    EXPECT_EQ(cf.gradient(elem_point, current_time)(0),
              raw_ad_cf.gradient(elem_point, current_time)(0));
    EXPECT_EQ(cf.gradient(elem_qp, current_time)(0), raw_ad_cf.gradient(elem_qp, current_time)(0));
    EXPECT_EQ(cf.gradient(elem_side_qp, current_time)(0),
              raw_ad_cf.gradient(elem_side_qp, current_time)(0));
    EXPECT_EQ(cf.dot(elem_arg, current_time), raw_ad_cf.dot(elem_arg, current_time));
    EXPECT_EQ(cf.dot(face, current_time), raw_ad_cf.dot(face, current_time));
    EXPECT_EQ(cf.dot(elem_point, current_time), raw_ad_cf.dot(elem_point, current_time));
    EXPECT_EQ(cf.dot(elem_qp, current_time), raw_ad_cf.dot(elem_qp, current_time));
    EXPECT_EQ(cf.dot(elem_side_qp, current_time), raw_ad_cf.dot(elem_side_qp, current_time));
  }

  const char * argv[2] = {"foo", "\0"};

  MultiMooseEnum coord_type_enum("XYZ RZ RSPHERICAL", "XYZ");

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
  mesh->setCoordSystem({}, coord_type_enum);
  // Build the face info
  const auto & all_fi = mesh->allFaceInfo();
  mesh->computeFaceInfoFaceCoords();

  // Test VectorComponentFunctor
  {
    WithGradientTestFunctor<RealVectorValue> vec_test_func(*mesh);
    VectorComponentFunctor<Real> vec_comp(vec_test_func, 0);
    EXPECT_EQ(vec_comp(elem_arg, current_time), 0);
    EXPECT_EQ(vec_comp(face, current_time), 0);
    EXPECT_EQ(vec_comp(elem_qp, current_time), 0);
    EXPECT_EQ(vec_comp(elem_side_qp, current_time), 0);
    EXPECT_EQ(vec_comp(elem_point, current_time), 0);

    bool found_internal = false;
    for (const auto & mesh_fi : all_fi)
    {
      if (!mesh_fi.neighborPtr())
        continue;

      auto vec_face_arg = FaceArg({&mesh_fi, LimiterType::CentralDifference, true, false, nullptr});
      const auto vec_elem_arg = vec_face_arg.makeElem();
      const auto vec_neighbor_arg = vec_face_arg.makeNeighbor();
      zero_gradient_test(vec_comp, vec_elem_arg);
      zero_gradient_test(vec_comp, vec_neighbor_arg);
      zero_gradient_test(vec_comp, vec_face_arg);

      found_internal = true;
      break;
    }

    EXPECT_TRUE(found_internal);
  }

  // Test NullFunctor errors
  {
    NullFunctor<Real> null;
    auto test_null_error = [&null, &current_time](const auto & arg)
    {
      try
      {
        null(arg, current_time);
        EXPECT_TRUE(false);
      }
      catch (std::runtime_error & e)
      {
        EXPECT_TRUE(std::string(e.what()).find("should never get here") != std::string::npos);
      }
    };

    test_null_error(elem_arg);
    test_null_error(face);
    test_null_error(elem_qp);
    test_null_error(elem_side_qp);
    test_null_error(elem_point);
  }

  // Test PiecewiseByBlockLambdaFunctor
  {
    // Test subdomain error
    {
      auto dummy_lammy = [](const auto &, const auto &) -> Real { return 2; };
      BypassFaceError<Real> errorful("errorful", dummy_lammy, {EXEC_ALWAYS}, *mesh, {});
      BypassFaceError<Real> errorfree(
          "errorfree", dummy_lammy, {EXEC_ALWAYS}, *mesh, mesh->meshSubdomains());

      auto test_sub_error = [&errorful, &current_time](const auto & arg)
      {
        try
        {
          errorful(arg, current_time);
          EXPECT_TRUE(false);
        }
        catch (std::runtime_error & e)
        {
          std::string error_message(e.what());
          EXPECT_TRUE(error_message.find(
                          "did not provide a functor material definition on that subdomain") !=
                      std::string::npos);
        }
      };

      test_sub_error(elem_arg);
      face.face_side = elem.get();
      test_sub_error(face);
      face.face_side = nullptr;
      test_sub_error(elem_qp);
      test_sub_error(elem_side_qp);
      test_sub_error(elem_point);
      EXPECT_EQ(errorfree(elem_point, current_time), 2);
    }

    // Test functions
    {
      auto zero_lammy = [](const auto &, const auto &) -> Real { return 0; };
      PiecewiseByBlockLambdaFunctor<Real> zero(
          "zero", zero_lammy, {EXEC_ALWAYS}, *mesh, {mesh->meshSubdomains()});

      for (const auto & mesh_elem : mesh->getMesh().active_element_ptr_range())
      {
        const auto mesh_elem_arg = ElemArg{mesh_elem, false};
        zero_gradient_test(zero, mesh_elem_arg);
      }

      for (const auto & mesh_fi : all_fi)
        EXPECT_TRUE(zero.isExtrapolatedBoundaryFace(mesh_fi, nullptr, current_time) ==
                    !(mesh_fi.neighborPtr()));
    }
  }
}
