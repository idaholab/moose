#ifdef MFEM_ENABLED

#include <algorithm>

#include "gtest/gtest.h"
#include "PropertyManager.h"

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

class CheckPropertyManager : public testing::Test
{
protected:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point1, point2;
  Moose::MFEM::ScalarCoefficientManager _scalar_manager;
  Moose::MFEM::VectorCoefficientManager _vector_manager;
  Moose::MFEM::MatrixCoefficientManager _matrix_manager;
  Moose::MFEM::PropertyManager manager;
  CheckPropertyManager() : manager(_scalar_manager, _vector_manager, _matrix_manager)
  {
    point1.Init(2);
    point1.Set2(0., 0.);
    point2.Init(2);
    point2.Set2(0.5, 1.);
    fe_transform.SetIdentityTransformation(mfem::Geometry::SQUARE);
  }
};

mfem::real_t
scalar_func(const mfem::Vector & x)
{
  return x[0] + x[1];
}

void
vector_func(const mfem::Vector & x, mfem::Vector & vec)
{
  vec[0] = 2 * x[0];
  vec[1] = x[1] + 1;
}

void
matrix_func(const mfem::Vector & x, mfem::DenseMatrix & mat)
{
  mat(0, 0) = 2 * x[0];
  mat(0, 1) = x[1] + 1;
  mat(1, 0) = 0.;
  mat(1, 1) = x[0] + x[1];
}

mfem::real_t
scalar_func_t(const mfem::Vector & x, mfem::real_t t)
{
  return x[0] + x[1] + t;
}

void
vector_func_t(const mfem::Vector & x, mfem::real_t t, mfem::Vector & vec)
{
  vec[0] = 2 * x[0] + t;
  vec[1] = x[1] + 1;
}

void
matrix_func_t(const mfem::Vector & x, mfem::real_t t, mfem::DenseMatrix & mat)
{
  mat(0, 0) = 2 * x[0] + t;
  mat(0, 1) = x[1] + 1;
  mat(1, 0) = t;
  mat(1, 1) = x[0] + x[1];
}

TEST_F(CheckPropertyManager, DeclareUniformScalar)
{
  manager.declareScalar<mfem::ConstantCoefficient>("resistivity", manager.global, 2.);
  mfem::ConstantCoefficient * c =
      dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
}

TEST_F(CheckPropertyManager, DeclarePWScalar)
{
  manager.declareScalar<mfem::ConstantCoefficient>("test", {"1", "2"}, 2.);
  manager.declareScalar<mfem::ConstantCoefficient>("test", {"3"}, 1.);
  mfem::PWCoefficient * c = dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarProperty("test"));
  ASSERT_NE(c, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  fe_transform.Attribute = 3;
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  fe_transform.Attribute = 10;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
}

TEST_F(CheckPropertyManager, DeclareFunctionScalar)
{
  manager.declareScalar<mfem::FunctionCoefficient>("resistivity", manager.global, scalar_func);
  auto c = &manager.getScalarProperty("resistivity");
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
}

TEST_F(CheckPropertyManager, DeclareFunctionTScalar)
{
  manager.declareScalar<mfem::FunctionCoefficient>("resistivity", manager.global, scalar_func_t);
  auto c = &manager.getScalarProperty("resistivity");
  c->SetTime(0.);
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
  c->SetTime(1.);
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.5);
}

TEST_F(CheckPropertyManager, DeclareFunctionPWScalar)
{
  manager.declareScalar<mfem::FunctionCoefficient>("test", {"1", "2"}, scalar_func);
  manager.declareScalar<mfem::FunctionCoefficient>(
      "test", {"3"}, [](const mfem::Vector & x) -> mfem::real_t { return scalar_func(x) + 1.; });
  mfem::PWCoefficient * c = dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarProperty("test"));
  ASSERT_NE(c, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 3;
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.5);
  fe_transform.Attribute = 10;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 0.0);
}

TEST_F(CheckPropertyManager, DeclareFunctionTPWScalar)
{
  manager.declareScalar<mfem::FunctionCoefficient>("test", {"1", "2"}, scalar_func_t);
  manager.declareScalar<mfem::FunctionCoefficient>(
      "test", {"3"}, [](const mfem::Vector & x) -> mfem::real_t { return scalar_func(x) + 1.; });
  mfem::PWCoefficient * c = dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarProperty("test"));
  ASSERT_NE(c, nullptr);
  c->SetTime(0.);
  fe_transform.Attribute = 1;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 3;
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.5);
  fe_transform.Attribute = 10;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 0.0);
  c->SetTime(2.);
  fe_transform.Attribute = 1;
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 3.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 3.5);
  fe_transform.Attribute = 3;
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.5);
  fe_transform.Attribute = 10;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 0.0);
}

TEST_F(CheckPropertyManager, DeclareCoefficientScalar)
{
  manager.declareScalar("resistivity", _scalar_manager.make<mfem::ConstantCoefficient>(2.));
  manager.declareScalar(
      "permittivity", manager.global, _scalar_manager.make<mfem::ConstantCoefficient>(3.));
  mfem::ConstantCoefficient * c =
      dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
  c = dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarProperty("permittivity"));
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 3.0);
}

TEST_F(CheckPropertyManager, DeclareCoefficientPWScalar)
{
  manager.declareScalar("test", {"1", "2"}, _scalar_manager.make<mfem::ConstantCoefficient>(2.));
  manager.declareScalar("test", {"3"}, _scalar_manager.make<mfem::ConstantCoefficient>(1.));
  mfem::PWCoefficient * c = dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarProperty("test"));
  ASSERT_NE(c, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  fe_transform.Attribute = 3;
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  fe_transform.Attribute = 10;
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
}

TEST_F(CheckPropertyManager, ScalarIsDefined)
{
  manager.declareScalar<mfem::ConstantCoefficient>("a", manager.global, 2.);
  manager.declareScalar<mfem::FunctionCoefficient>("b", manager.global, scalar_func);
  manager.declareScalar("c", _scalar_manager.make<mfem::ConstantCoefficient>(2.));
  EXPECT_TRUE(manager.scalarIsDefined("a", "1"));
  EXPECT_TRUE(manager.scalarIsDefined("a", "10"));
  EXPECT_FALSE(manager.scalarIsDefined("A", "1"));
  EXPECT_TRUE(manager.scalarIsDefined("b", "1"));
  EXPECT_TRUE(manager.scalarIsDefined("b", "-57"));
  EXPECT_FALSE(manager.scalarIsDefined("B", "1"));
  EXPECT_TRUE(manager.scalarIsDefined("c", "0"));
  EXPECT_TRUE(manager.scalarIsDefined("c", "20"));
  EXPECT_FALSE(manager.scalarIsDefined("C", "0"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "0"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "2"));

  EXPECT_FALSE(manager.vectorIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("b", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("b", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("c", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("c", "1"));
}

TEST_F(CheckPropertyManager, ScalarPWIsDefined)
{
  manager.declareScalar<mfem::ConstantCoefficient>("a", {"1", "2"}, 2.);
  manager.declareScalar<mfem::FunctionCoefficient>("b", {"-1", "0"}, scalar_func);
  manager.declareScalar("c", {"42", "45"}, _scalar_manager.make<mfem::ConstantCoefficient>(2.));
  EXPECT_TRUE(manager.scalarIsDefined("a", "1"));
  EXPECT_TRUE(manager.scalarIsDefined("a", "2"));
  EXPECT_FALSE(manager.scalarIsDefined("a", "0"));
  EXPECT_FALSE(manager.scalarIsDefined("A", "1"));
  EXPECT_TRUE(manager.scalarIsDefined("b", "-1"));
  EXPECT_TRUE(manager.scalarIsDefined("b", "0"));
  EXPECT_FALSE(manager.scalarIsDefined("b", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("B", "0"));
  EXPECT_TRUE(manager.scalarIsDefined("c", "42"));
  EXPECT_TRUE(manager.scalarIsDefined("c", "45"));
  EXPECT_FALSE(manager.scalarIsDefined("c", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("C", "42"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "-1"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "0"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("d", "2"));

  EXPECT_FALSE(manager.vectorIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("b", "-1"));
  EXPECT_FALSE(manager.matrixIsDefined("b", "-1"));
  EXPECT_FALSE(manager.vectorIsDefined("c", "42"));
  EXPECT_FALSE(manager.matrixIsDefined("c", "42"));
}

TEST_F(CheckPropertyManager, DeclareUniformVector)
{
  manager.declareVector<mfem::VectorConstantCoefficient>(
      "resistivity", manager.global, mfem::Vector({1., 2.}));
  mfem::VectorConstantCoefficient * c =
      dynamic_cast<mfem::VectorConstantCoefficient *>(&manager.getVectorProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
}

TEST_F(CheckPropertyManager, DeclarePWVector)
{
  manager.declareVector<mfem::VectorConstantCoefficient>(
      "test", {"1", "2"}, mfem::Vector({1., 2.}));
  manager.declareVector<mfem::VectorConstantCoefficient>("test", {"3"}, mfem::Vector({3., 4.}));
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  fe_transform.Attribute = 1;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  fe_transform.Attribute = 2;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  fe_transform.Attribute = 3;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 3.0);
  EXPECT_EQ(vec[1], 4.0);
  fe_transform.Attribute = 10;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 0.);
}

TEST_F(CheckPropertyManager, DeclareFunctionVector)
{
  manager.declareVector<mfem::VectorFunctionCoefficient>(
      "resistivity", manager.global, 2, vector_func);
  auto c = &manager.getVectorProperty("resistivity");
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 1.);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.);
}

TEST_F(CheckPropertyManager, DeclareFunctionTVector)
{
  manager.declareVector<mfem::VectorFunctionCoefficient>(
      "resistivity", manager.global, 2, vector_func_t);
  auto c = &manager.getVectorProperty("resistivity");
  mfem::Vector vec;
  c->SetTime(0.);
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 1.);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.);
  c->SetTime(1.);
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 1.);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 2.);
  EXPECT_EQ(vec[1], 2.);
}

TEST_F(CheckPropertyManager, DeclareFunctionPWVector)
{
  manager.declareVector<mfem::VectorFunctionCoefficient>("test", {"1", "2"}, 2, vector_func);
  manager.declareVector<mfem::VectorFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::Vector & vec)
      {
        vector_func(x, vec);
        vec *= scalar_func(x);
      });
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  fe_transform.Attribute = 1;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 1.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.0);
  fe_transform.Attribute = 2;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 1.0);
  fe_transform.Attribute = 3;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 0.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.5);
  EXPECT_EQ(vec[1], 3.0);
  fe_transform.Attribute = 10;
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 0.);
}
TEST_F(CheckPropertyManager, DeclareFunctionTPWVector)
{
  manager.declareVector<mfem::VectorFunctionCoefficient>("test", {"1", "2"}, 2, vector_func_t);
  manager.declareVector<mfem::VectorFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::Vector & vec)
      {
        vector_func(x, vec);
        vec *= scalar_func(x);
      });
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  c->SetTime(0.);
  fe_transform.Attribute = 1;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 1.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.0);
  fe_transform.Attribute = 2;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 1.0);
  fe_transform.Attribute = 3;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 0.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.5);
  EXPECT_EQ(vec[1], 3.0);
  fe_transform.Attribute = 10;
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 0.);
  c->SetTime(1.);
  fe_transform.Attribute = 1;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 1.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 2.);
  EXPECT_EQ(vec[1], 2.0);
  fe_transform.Attribute = 2;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 1.0);
  fe_transform.Attribute = 3;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 0.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.5);
  EXPECT_EQ(vec[1], 3.0);
  fe_transform.Attribute = 10;
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 0.);
}

TEST_F(CheckPropertyManager, DeclareCoefficientVector)
{
  manager.declareVector(
      "resistivity",
      manager.global,
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({1., 2.})));
  manager.declareVector(
      "permittivity",
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({3., 4.})));
  mfem::VectorConstantCoefficient * c =
      dynamic_cast<mfem::VectorConstantCoefficient *>(&manager.getVectorProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.0);
  c = dynamic_cast<mfem::VectorConstantCoefficient *>(&manager.getVectorProperty("permittivity"));
  ASSERT_NE(c, nullptr);
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 3.0);
  EXPECT_EQ(vec[1], 4.0);
}

TEST_F(CheckPropertyManager, DeclareCoefficientPWVector)
{
  manager.declareVector(
      "test",
      {"1", "2"},
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({2., 1.})));
  manager.declareVector(
      "test",
      {"3"},
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({-1., -7.})));
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  fe_transform.Attribute = 1;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 2.0);
  EXPECT_EQ(vec[1], 1.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 2.0);
  EXPECT_EQ(vec[1], 1.0);
  fe_transform.Attribute = 2;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 2.0);
  EXPECT_EQ(vec[1], 1.0);
  fe_transform.Attribute = 3;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], -1.0);
  EXPECT_EQ(vec[1], -7.0);
  fe_transform.Attribute = 10;
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 0.0);
}

TEST_F(CheckPropertyManager, VectorIsDefined)
{
  manager.declareVector<mfem::VectorConstantCoefficient>(
      "a", manager.global, mfem::Vector({2., 1.}));
  manager.declareVector<mfem::VectorFunctionCoefficient>("b", manager.global, 2, vector_func);
  manager.declareVector(
      "c", _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({3., 4.})));
  EXPECT_TRUE(manager.vectorIsDefined("a", "1"));
  EXPECT_TRUE(manager.vectorIsDefined("a", "10"));
  EXPECT_FALSE(manager.vectorIsDefined("A", "1"));
  EXPECT_TRUE(manager.vectorIsDefined("b", "1"));
  EXPECT_TRUE(manager.vectorIsDefined("b", "-57"));
  EXPECT_FALSE(manager.vectorIsDefined("B", "1"));
  EXPECT_TRUE(manager.vectorIsDefined("c", "0"));
  EXPECT_TRUE(manager.vectorIsDefined("c", "20"));
  EXPECT_FALSE(manager.vectorIsDefined("C", "0"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "0"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("b", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("b", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("c", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("c", "1"));
}

TEST_F(CheckPropertyManager, VectorPWIsDefined)
{
  manager.declareVector<mfem::VectorConstantCoefficient>("a", {"1", "2"}, mfem::Vector({2., 1.}));
  manager.declareVector<mfem::VectorFunctionCoefficient>("b", {"-1", "0"}, 2, vector_func);
  manager.declareVector(
      "c",
      {"42", "45"},
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({3., 4.})));
  EXPECT_TRUE(manager.vectorIsDefined("a", "1"));
  EXPECT_TRUE(manager.vectorIsDefined("a", "2"));
  EXPECT_FALSE(manager.vectorIsDefined("a", "0"));
  EXPECT_FALSE(manager.vectorIsDefined("A", "1"));
  EXPECT_TRUE(manager.vectorIsDefined("b", "-1"));
  EXPECT_TRUE(manager.vectorIsDefined("b", "0"));
  EXPECT_FALSE(manager.vectorIsDefined("b", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("B", "0"));
  EXPECT_TRUE(manager.vectorIsDefined("c", "42"));
  EXPECT_TRUE(manager.vectorIsDefined("c", "45"));
  EXPECT_FALSE(manager.vectorIsDefined("c", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("C", "42"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "-1"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "0"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("b", "-1"));
  EXPECT_FALSE(manager.matrixIsDefined("b", "-1"));
  EXPECT_FALSE(manager.scalarIsDefined("c", "42"));
  EXPECT_FALSE(manager.matrixIsDefined("c", "42"));
}

TEST_F(CheckPropertyManager, DeclareUniformMatrix)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "resistivity", manager.global, mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  mfem::MatrixConstantCoefficient * c =
      dynamic_cast<mfem::MatrixConstantCoefficient *>(&manager.getMatrixProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
}

TEST_F(CheckPropertyManager, DeclarePWMatrix)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "test", {"1", "2"}, mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "test", {"3"}, mfem::DenseMatrix({{-1., 4.}, {-10., -2.}}));
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  fe_transform.Attribute = 1;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  fe_transform.Attribute = 2;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  fe_transform.Attribute = 3;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), -1.0);
  EXPECT_EQ(mat.Elem(0, 1), 4.0);
  EXPECT_EQ(mat.Elem(1, 0), -10.0);
  EXPECT_EQ(mat.Elem(1, 1), -2.0);
  fe_transform.Attribute = 10;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
}

TEST_F(CheckPropertyManager, DeclareFunctionMatrix)
{
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>(
      "resistivity", manager.global, 2, matrix_func);
  auto c = &manager.getMatrixProperty("resistivity");
  mfem::DenseMatrix mat;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 1.5);
}

TEST_F(CheckPropertyManager, DeclareFunctionTMatrix)
{
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>(
      "resistivity", manager.global, 2, matrix_func_t);
  auto c = &manager.getMatrixProperty("resistivity");
  mfem::DenseMatrix mat;
  c->SetTime(0.0);
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 1.5);
  c->SetTime(5.0);
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 5.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 5.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 6.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 5.0);
  EXPECT_EQ(mat.Elem(1, 1), 1.5);
}

TEST_F(CheckPropertyManager, DeclareFunctionPWMatrix)
{
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("test", {"1", "2"}, 2, matrix_func);
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::DenseMatrix & mat)
      {
        matrix_func(x, mat);
        mat *= scalar_func(x);
      });
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  fe_transform.Attribute = 1;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 1.5);
  fe_transform.Attribute = 2;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  fe_transform.Attribute = 3;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.5);
  EXPECT_EQ(mat.Elem(0, 1), 3.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 2.25);
  fe_transform.Attribute = 10;
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
}

TEST_F(CheckPropertyManager, DeclareFunctionTPWMatrix)
{
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("test", {"1", "2"}, 2, matrix_func_t);
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::DenseMatrix & mat)
      {
        matrix_func(x, mat);
        mat *= scalar_func(x);
      });
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  c->SetTime(0.);
  fe_transform.Attribute = 1;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 1.5);
  fe_transform.Attribute = 2;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  fe_transform.Attribute = 3;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.5);
  EXPECT_EQ(mat.Elem(0, 1), 3.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 2.25);
  fe_transform.Attribute = 10;
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->SetTime(-1.);
  fe_transform.Attribute = 1;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), -1.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), -1.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), -1.0);
  EXPECT_EQ(mat.Elem(1, 1), 1.5);
  fe_transform.Attribute = 2;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), -1.);
  EXPECT_EQ(mat.Elem(0, 1), 1.);
  EXPECT_EQ(mat.Elem(1, 0), -1.);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  fe_transform.Attribute = 3;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.5);
  EXPECT_EQ(mat.Elem(0, 1), 3.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 2.25);
  fe_transform.Attribute = 10;
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
}

TEST_F(CheckPropertyManager, DeclareCoefficientMatrix)
{
  manager.declareMatrix("resistivity",
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{1., 2.}, {3., 4.}})));
  manager.declareMatrix("permittivity",
                        manager.global,
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{5., 6.}, {7., 8.}})));
  mfem::MatrixConstantCoefficient * c =
      dynamic_cast<mfem::MatrixConstantCoefficient *>(&manager.getMatrixProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  c = dynamic_cast<mfem::MatrixConstantCoefficient *>(&manager.getMatrixProperty("permittivity"));
  ASSERT_NE(c, nullptr);
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 5.0);
  EXPECT_EQ(mat.Elem(0, 1), 6.0);
  EXPECT_EQ(mat.Elem(1, 0), 7.0);
  EXPECT_EQ(mat.Elem(1, 1), 8.0);
}

TEST_F(CheckPropertyManager, DeclareCoefficientPWMatrix)
{
  manager.declareMatrix("test",
                        {"1", "2"},
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{1., 2.}, {3., 4.}})));
  manager.declareMatrix("test",
                        {"3"},
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{-1., 4.}, {-10., -2.}})));
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixProperty("test"));
  ASSERT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  fe_transform.Attribute = 1;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  c->Eval(mat, fe_transform, point2);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  fe_transform.Attribute = 2;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.0);
  EXPECT_EQ(mat.Elem(0, 1), 2.0);
  EXPECT_EQ(mat.Elem(1, 0), 3.0);
  EXPECT_EQ(mat.Elem(1, 1), 4.0);
  fe_transform.Attribute = 3;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), -1.0);
  EXPECT_EQ(mat.Elem(0, 1), 4.0);
  EXPECT_EQ(mat.Elem(1, 0), -10.0);
  EXPECT_EQ(mat.Elem(1, 1), -2.0);
  fe_transform.Attribute = 10;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.0);
  EXPECT_EQ(mat.Elem(0, 1), 0.0);
  EXPECT_EQ(mat.Elem(1, 0), 0.0);
  EXPECT_EQ(mat.Elem(1, 1), 0.0);
}

TEST_F(CheckPropertyManager, MatrixIsDefined)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "a", manager.global, mfem::DenseMatrix({{2., 1.}, {0., 0.}}));
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("b", manager.global, 2, matrix_func);
  manager.declareMatrix("c",
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{1., 2.}, {3., 4.}})));
  EXPECT_TRUE(manager.matrixIsDefined("a", "1"));
  EXPECT_TRUE(manager.matrixIsDefined("a", "10"));
  EXPECT_FALSE(manager.matrixIsDefined("A", "1"));
  EXPECT_TRUE(manager.matrixIsDefined("b", "1"));
  EXPECT_TRUE(manager.matrixIsDefined("b", "-57"));
  EXPECT_FALSE(manager.matrixIsDefined("B", "1"));
  EXPECT_TRUE(manager.matrixIsDefined("c", "0"));
  EXPECT_TRUE(manager.matrixIsDefined("c", "20"));
  EXPECT_FALSE(manager.matrixIsDefined("C", "0"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "0"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("b", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("b", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("c", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("c", "1"));
}

TEST_F(CheckPropertyManager, MatrixPWIsDefined)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "a", {"1", "2"}, mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("b", {"-1", "0"}, 2, matrix_func);
  manager.declareMatrix("c",
                        {"42", "45"},
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{1., 2.}, {3., 4.}})));
  EXPECT_TRUE(manager.matrixIsDefined("a", "1"));
  EXPECT_TRUE(manager.matrixIsDefined("a", "2"));
  EXPECT_FALSE(manager.matrixIsDefined("a", "0"));
  EXPECT_FALSE(manager.matrixIsDefined("A", "1"));
  EXPECT_TRUE(manager.matrixIsDefined("b", "-1"));
  EXPECT_TRUE(manager.matrixIsDefined("b", "0"));
  EXPECT_FALSE(manager.matrixIsDefined("b", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("B", "0"));
  EXPECT_TRUE(manager.matrixIsDefined("c", "42"));
  EXPECT_TRUE(manager.matrixIsDefined("c", "45"));
  EXPECT_FALSE(manager.matrixIsDefined("c", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("C", "42"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "-1"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "0"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "1"));
  EXPECT_FALSE(manager.matrixIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarIsDefined("b", "-1"));
  EXPECT_FALSE(manager.vectorIsDefined("b", "-1"));
  EXPECT_FALSE(manager.scalarIsDefined("c", "42"));
  EXPECT_FALSE(manager.vectorIsDefined("c", "42"));
}

TEST_F(CheckPropertyManager, CheckRepeatedNames)
{
  // Check there can be scalar, vector, and matrix coefficients defined with the same name
  manager.declareScalar<mfem::ConstantCoefficient>("a", manager.global, 2.);
  manager.declareVector<mfem::VectorConstantCoefficient>(
      "a", manager.global, mfem::Vector({2., 1.}));
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "a", manager.global, mfem::DenseMatrix({{2., 1.}, {0., 1.}}));

  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "b", manager.global, mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareScalar<mfem::ConstantCoefficient>("b", manager.global, 2.);
  manager.declareVector<mfem::VectorConstantCoefficient>(
      "b", manager.global, mfem::Vector({2., 1.}));

  manager.declareVector<mfem::VectorConstantCoefficient>(
      "c", manager.global, mfem::Vector({2., 1.}));
  manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "c", manager.global, mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareScalar<mfem::ConstantCoefficient>("c", manager.global, 2.);

  // Check that coefficients can not be redefined
  EXPECT_THROW(manager.declareScalar<mfem::ConstantCoefficient>("a", manager.global, 2.),
               MooseException);
  EXPECT_THROW(manager.declareVector<mfem::VectorConstantCoefficient>(
                   "a", manager.global, mfem::Vector({2., 1.})),
               MooseException);
  EXPECT_THROW(manager.declareMatrix<mfem::MatrixConstantCoefficient>(
                   "a", manager.global, mfem::DenseMatrix({{2., 1.}, {0., 1.}})),
               MooseException);
}

#endif
