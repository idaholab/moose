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
#include "libmesh/elem.h"
#include "libmesh/quadrature_gauss.h"

using namespace libMesh;
using namespace Moose;

template <typename T>
class TestFunctor : public FunctorBase<T>
{
public:
  using typename FunctorBase<T>::FunctorType;
  using typename FunctorBase<T>::FunctorReturnType;
  using typename FunctorBase<T>::ValueType;
  using typename FunctorBase<T>::GradientType;
  using typename FunctorBase<T>::DotType;

  TestFunctor() = default;

private:
  ValueType evaluate(const Moose::ElemArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const Moose::ElemFromFaceArg &, unsigned int) const override final
  {
    return 0;
  }
  ValueType evaluate(const Moose::FaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const Moose::SingleSidedFaceArg &, unsigned int) const override final
  {
    return 0;
  }
  ValueType evaluate(const Moose::ElemQpArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const Moose::ElemSideQpArg &, unsigned int) const override final { return 0; }
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

  auto elem_arg = Moose::ElemArg{elem.get(), false, false};
  auto face = Moose::FaceArg({&fi,
                              FV::LimiterType::CentralDifference,
                              true,
                              false,
                              false,
                              INVALID_BLOCK_ID,
                              INVALID_BLOCK_ID});
  auto single_face = Moose::SingleSidedFaceArg(
      {&fi, FV::LimiterType::CentralDifference, true, false, false, INVALID_BLOCK_ID});
  auto elem_from_face = Moose::ElemFromFaceArg({elem.get(), &fi, false, false, INVALID_BLOCK_ID});
  auto elem_qp = std::make_tuple(elem.get(), 0, &qrule);
  auto elem_side_qp = std::make_tuple(elem.get(), 0, 0, &qrule);

  auto test_dot = [&test](const auto & arg)
  {
    try
    {
      test.dot(arg);
      ASSERT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      ASSERT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
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
      ASSERT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      ASSERT_TRUE(std::string(e.what()).find("not implemented") != std::string::npos);
    }
  };

  test_gradient(elem_arg);
  test_gradient(face);
  test_gradient(single_face);
  test_gradient(elem_from_face);
  test_gradient(elem_qp);
  test_gradient(elem_side_qp);

  ConstantFunctor<Real> cf(2);
  EXPECT_EQ(cf(elem_arg), 2);
  EXPECT_EQ(cf(elem_from_face), 2);
  EXPECT_EQ(cf(face), 2);
  EXPECT_EQ(cf(single_face), 2);
  EXPECT_EQ(cf(elem_from_face), 2);
  EXPECT_EQ(cf(elem_qp), 2);
  EXPECT_EQ(cf(elem_side_qp), 2);

  auto constant_gradient_test = [&cf](const auto & arg)
  {
    const auto result = cf.gradient(arg);
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
      EXPECT_EQ(result(i), 0);
  };
  constant_gradient_test(elem_arg);
  constant_gradient_test(elem_from_face);
  constant_gradient_test(face);
  constant_gradient_test(single_face);
  constant_gradient_test(elem_from_face);
  constant_gradient_test(elem_qp);
  constant_gradient_test(elem_side_qp);

  EXPECT_EQ(cf.dot(elem_arg), 0);
  EXPECT_EQ(cf.dot(elem_from_face), 0);
  EXPECT_EQ(cf.dot(face), 0);
  EXPECT_EQ(cf.dot(single_face), 0);
  EXPECT_EQ(cf.dot(elem_from_face), 0);
  EXPECT_EQ(cf.dot(elem_qp), 0);
  EXPECT_EQ(cf.dot(elem_side_qp), 0);
}
