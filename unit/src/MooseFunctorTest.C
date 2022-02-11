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
#include "VectorComponentFunctor.h"
#include "GreenGaussGradient.h"
#include "MeshGeneratorMesh.h"
#include "GeneratedMeshGenerator.h"
#include "AppFactory.h"
#include "PiecewiseByBlockLambdaFunctor.h"
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

  TestFunctor() = default;

private:
  ValueType evaluate(const ElemArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const ElemFromFaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const FaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const SingleSidedFaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const ElemQpArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const ElemSideQpArg &, unsigned int) const override final { return 0; }
};

template <typename T>
class WithGradientTestFunctor : public TestFunctor<T>
{
public:
  using typename TestFunctor<T>::GradientType;

  WithGradientTestFunctor(const MooseMesh & mesh) : _mesh(mesh) {}

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi) const override { return !fi.neighborPtr(); }

private:
  using TestFunctor<T>::evaluateGradient;
  GradientType evaluateGradient(const ElemArg & elem_arg, unsigned int) const override final
  {
    return greenGaussGradient(elem_arg, *this, true, _mesh);
  }

  GradientType evaluateGradient(const FaceArg & face, unsigned int) const override final
  {
    const auto & fi = *face.fi;
    if (!isExtrapolatedBoundaryFace(fi))
    {
      const auto elem_arg = face.makeElem();
      const auto elem_gradient = this->gradient(elem_arg);
      const auto neighbor_arg = face.makeNeighbor();
      const auto linear_interp_gradient =
          fi.gC() * elem_gradient + (1 - fi.gC()) * this->gradient(neighbor_arg);
      return linear_interp_gradient +
             outer_product(((*this)(neighbor_arg) - (*this)(elem_arg)) / fi.dCFMag() -
                               linear_interp_gradient * fi.eCF(),
                           fi.eCF());
    }

    // One term expansion
    if (!fi.neighborPtr())
      return this->gradient(face.makeElem());
    else
      return this->gradient(face.makeNeighbor());
  }

  const MooseMesh & _mesh;
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
  FaceInfo fi(elem.get(), 1, neighbor.get());
  QGauss qrule(1, CONSTANT);

  auto elem_arg = ElemArg{elem.get(), false, false};
  auto face = FaceArg({&fi,
                       LimiterType::CentralDifference,
                       true,
                       false,
                       false,
                       INVALID_BLOCK_ID,
                       INVALID_BLOCK_ID});
  auto single_face = SingleSidedFaceArg(
      {&fi, LimiterType::CentralDifference, true, false, false, INVALID_BLOCK_ID});
  auto elem_from_face = ElemFromFaceArg({elem.get(), &fi, false, false, INVALID_BLOCK_ID});
  auto elem_qp = std::make_tuple(elem.get(), 0, &qrule);
  auto elem_side_qp = std::make_tuple(elem.get(), 0, 0, &qrule);

  // Test not-implemented errors
  {
    auto test_dot = [&test](const auto & arg)
    {
      try
      {
        test.dot(arg);
        EXPECT_TRUE(false);
      }
      catch (std::runtime_error & e)
      {
        EXPECT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
      }
    };

    test_dot(elem_arg);
    test_dot(face);
    test_dot(single_face);
    test_dot(elem_from_face);
    test_dot(elem_qp);
    test_dot(elem_side_qp);

    auto test_gradient = [&test](const auto & arg)
    {
      try
      {
        test.gradient(arg);
        EXPECT_TRUE(false);
      }
      catch (std::runtime_error & e)
      {
        EXPECT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
      }
    };

    test_gradient(elem_arg);
    test_gradient(face);
    test_gradient(single_face);
    test_gradient(elem_from_face);
    test_gradient(elem_qp);
    test_gradient(elem_side_qp);
  }

  auto zero_gradient_test = [](const auto & functor, const auto & arg)
  {
    const auto result = functor.gradient(arg);
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
      EXPECT_EQ(result(i), 0);
  };

  // Test ConstantFunctor
  {
    ConstantFunctor<Real> cf(2);
    EXPECT_EQ(cf(elem_arg), 2);
    EXPECT_EQ(cf(elem_from_face), 2);
    EXPECT_EQ(cf(face), 2);
    EXPECT_EQ(cf(single_face), 2);
    EXPECT_EQ(cf(elem_from_face), 2);
    EXPECT_EQ(cf(elem_qp), 2);
    EXPECT_EQ(cf(elem_side_qp), 2);

    zero_gradient_test(cf, elem_arg);
    zero_gradient_test(cf, elem_from_face);
    zero_gradient_test(cf, face);
    zero_gradient_test(cf, single_face);
    zero_gradient_test(cf, elem_from_face);
    zero_gradient_test(cf, elem_qp);
    zero_gradient_test(cf, elem_side_qp);

    EXPECT_EQ(cf.dot(elem_arg), 0);
    EXPECT_EQ(cf.dot(elem_from_face), 0);
    EXPECT_EQ(cf.dot(face), 0);
    EXPECT_EQ(cf.dot(single_face), 0);
    EXPECT_EQ(cf.dot(elem_from_face), 0);
    EXPECT_EQ(cf.dot(elem_qp), 0);
    EXPECT_EQ(cf.dot(elem_side_qp), 0);
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
    EXPECT_EQ(vec_comp(elem_arg), 0);
    EXPECT_EQ(vec_comp(elem_from_face), 0);
    EXPECT_EQ(vec_comp(face), 0);
    EXPECT_EQ(vec_comp(single_face), 0);
    EXPECT_EQ(vec_comp(elem_from_face), 0);
    EXPECT_EQ(vec_comp(elem_qp), 0);
    EXPECT_EQ(vec_comp(elem_side_qp), 0);

    bool found_internal = false;
    for (const auto & mesh_fi : all_fi)
    {
      if (!mesh_fi.neighborPtr())
        continue;

      auto vec_face_arg =
          FaceArg({&mesh_fi,
                   LimiterType::CentralDifference,
                   true,
                   false,
                   false,
                   mesh_fi.elem().subdomain_id(),
                   mesh_fi.neighborPtr() ? mesh_fi.neighbor().subdomain_id() : INVALID_BLOCK_ID});
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
    auto test_null_error = [&null](const auto & arg)
    {
      try
      {
        null(arg);
        EXPECT_TRUE(false);
      }
      catch (std::runtime_error & e)
      {
        EXPECT_TRUE(std::string(e.what()).find("should never get here") != std::string::npos);
      }
    };

    test_null_error(elem_arg);
    test_null_error(face);
    test_null_error(single_face);
    test_null_error(elem_from_face);
    test_null_error(elem_qp);
    test_null_error(elem_side_qp);
  }

  // Test PiecewiseByBlockLambdaFunctor
  {
    // Test subdomain error
    {
      auto dummy_lammy = [](const auto &, const auto &) -> Real { return 0; };
      PiecewiseByBlockLambdaFunctor<Real> errorful(
          "errorful", dummy_lammy, {EXEC_ALWAYS}, *mesh, {});

      auto test_sub_error = [&errorful](const auto & arg)
      {
        try
        {
          errorful(arg);
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
      test_sub_error(face);
      test_sub_error(single_face);
      test_sub_error(elem_from_face);
      test_sub_error(elem_qp);
      test_sub_error(elem_side_qp);
    }

    // Test functions
    {
      auto zero_lammy = [](const auto &, const auto &) -> Real { return 0; };
      PiecewiseByBlockLambdaFunctor<Real> zero(
          "zero", zero_lammy, {EXEC_ALWAYS}, *mesh, {mesh->meshSubdomains()});

      for (const auto & mesh_elem : mesh->getMesh().active_element_ptr_range())
      {
        const auto mesh_elem_arg = ElemArg{mesh_elem, false, false};
        zero_gradient_test(zero, mesh_elem_arg);
      }

      for (const auto & mesh_fi : all_fi)
        EXPECT_TRUE(zero.isExtrapolatedBoundaryFace(mesh_fi) == !(mesh_fi.neighborPtr()));
    }
  }
}
