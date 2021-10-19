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
class TestFunctor : public Functor<T>
{
public:
  using typename Functor<T>::FaceArg;
  using typename Functor<T>::SingleSidedFaceArg;
  using typename Functor<T>::ElemFromFaceArg;
  using typename Functor<T>::ElemQpArg;
  using typename Functor<T>::ElemSideQpArg;
  using typename Functor<T>::FunctorType;
  using typename Functor<T>::FunctorReturnType;
  using typename Functor<T>::ValueType;
  using typename Functor<T>::GradientType;
  using typename Functor<T>::DotType;

  TestFunctor() = default;

private:
  ValueType evaluate(const libMesh::Elem * const &, unsigned int) const override final { return 0; }
  ValueType evaluate(const ElemFromFaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const FaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const SingleSidedFaceArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const ElemQpArg &, unsigned int) const override final { return 0; }
  ValueType evaluate(const ElemSideQpArg &, unsigned int) const override final { return 0; }
};

TEST(MooseFunctorTest, testErrors)
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

  auto face = std::make_tuple(&fi,
                              FV::LimiterType::CentralDifference,
                              true,
                              std::make_pair(INVALID_BLOCK_ID, INVALID_BLOCK_ID));
  auto single_face =
      std::make_tuple(&fi, FV::LimiterType::CentralDifference, true, INVALID_BLOCK_ID);
  auto elem_from_face = std::make_tuple(elem.get(), &fi, INVALID_BLOCK_ID);
  auto elem_qp = std::make_tuple(elem.get(), 0, &qrule);
  auto elem_side_qp = std::make_tuple(elem.get(), 0, 0, &qrule);

  auto test_dot = [&test](const auto & arg) {
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

  test_dot(elem.get());
  test_dot(face);
  test_dot(single_face);
  test_dot(elem_from_face);
  test_dot(elem_qp);
  test_dot(elem_side_qp);

  auto test_gradient = [&test](const auto & arg) {
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

  test_gradient(elem.get());
  test_gradient(face);
  test_gradient(single_face);
  test_gradient(elem_from_face);
  test_gradient(elem_qp);
  test_gradient(elem_side_qp);
}
