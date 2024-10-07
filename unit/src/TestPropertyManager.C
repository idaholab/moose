#include <algorithm>

#include "gtest/gtest.h"
#include "PropertyManager.h"

#include "mfem.hpp"

class CheckPropertyManager : public testing::Test
{
protected:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point1, point2;
  platypus::ScalarCoefficientManager _scalar_manager;
  platypus::VectorCoefficientManager _vector_manager;
  platypus::MatrixCoefficientManager _matrix_manager;
  platypus::PropertyManager manager;
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
  manager.declareScalar<mfem::ConstantCoefficient>("resistivity", {}, 2.);
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
  manager.declareScalar("resistivity", scalar_func);
  auto c = &manager.getScalarProperty("resistivity");
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
}

TEST_F(CheckPropertyManager, DeclareFunctionTScalar)
{
  manager.declareScalar("resistivity", scalar_func_t);
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
  manager.declareScalar("test", scalar_func, {"1", "2"});
  manager.declareScalar(
      "test", [](const mfem::Vector & x) -> mfem::real_t { return scalar_func(x) + 1.; }, {"3"});
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
  manager.declareScalar("test", scalar_func_t, {"1", "2"});
  manager.declareScalar(
      "test", [](const mfem::Vector & x) -> mfem::real_t { return scalar_func(x) + 1.; }, {"3"});
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
  mfem::ConstantCoefficient * c =
      dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarProperty("resistivity"));
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
}

TEST_F(CheckPropertyManager, DeclareCoefficientPWScalar)
{
  manager.declareScalar("test", _scalar_manager.make<mfem::ConstantCoefficient>(2.), {"1", "2"});
  manager.declareScalar("test", _scalar_manager.make<mfem::ConstantCoefficient>(1.), {"3"});
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
  manager.declareScalar("a", 2.);
  manager.declareScalar("b", scalar_func);
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
  manager.declareScalar("a", 2., {"1", "2"});
  manager.declareScalar("b", scalar_func, {"-1", "0"});
  manager.declareScalar("c", _scalar_manager.make<mfem::ConstantCoefficient>(2.), {"42", "45"});
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
  manager.declareVector("resistivity", mfem::Vector({1., 2.}));
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
  manager.declareVector("test", mfem::Vector({1., 2.}), {"1", "2"});
  manager.declareVector("test", mfem::Vector({3., 4.}), {"3"});
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
  manager.declareVector("resistivity", 2, vector_func);
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
  manager.declareVector("resistivity", 2, vector_func_t);
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
  manager.declareVector("test", 2, vector_func, {"1", "2"});
  manager.declareVector("test",
                        2,
                        [](const mfem::Vector & x, mfem::Vector & vec)
                        {
                          vector_func(x, vec);
                          vec *= scalar_func(x);
                        },
                        {"3"});
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
  manager.declareVector("test", 2, vector_func_t, {"1", "2"});
  manager.declareVector("test",
                        2,
                        [](const mfem::Vector & x, mfem::Vector & vec)
                        {
                          vector_func(x, vec);
                          vec *= scalar_func(x);
                        },
                        {"3"});
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
      "resistivity", _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({1., 2.})));
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
}

TEST_F(CheckPropertyManager, DeclareCoefficientPWVector)
{
  manager.declareVector(
      "test",
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({2., 1.})),
      {"1", "2"});
  manager.declareVector(
      "test",
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({-1., -7.})),
      {"3"});
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
  manager.declareVector("a", mfem::Vector({2., 1.}));
  manager.declareVector("b", 2, vector_func);
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
  manager.declareVector("a", mfem::Vector({2., 1.}), {"1", "2"});
  manager.declareVector("b", 2, vector_func, {"-1", "0"});
  manager.declareVector(
      "c",
      _vector_manager.make<mfem::VectorConstantCoefficient>(mfem::Vector({3., 4.})),
      {"42", "45"});
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
  manager.declareMatrix("resistivity", mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
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
  manager.declareMatrix("test", mfem::DenseMatrix({{1., 2.}, {3., 4.}}), {"1", "2"});
  manager.declareMatrix("test", mfem::DenseMatrix({{-1., 4.}, {-10., -2.}}), {"3"});
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
  manager.declareMatrix("resistivity", 2, matrix_func);
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
  manager.declareMatrix("resistivity", 2, matrix_func_t);
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
  manager.declareMatrix("test", 2, matrix_func, {"1", "2"});
  manager.declareMatrix("test",
                        2,
                        [](const mfem::Vector & x, mfem::DenseMatrix & mat)
                        {
                          matrix_func(x, mat);
                          mat *= scalar_func(x);
                        },
                        {"3"});
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
  manager.declareMatrix("test", 2, matrix_func_t, {"1", "2"});
  manager.declareMatrix("test",
                        2,
                        [](const mfem::Vector & x, mfem::DenseMatrix & mat)
                        {
                          matrix_func(x, mat);
                          mat *= scalar_func(x);
                        },
                        {"3"});
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

TEST_F(CheckPropertyManager, DeclareCoefficientPWMatrix)
{
  manager.declareMatrix("test",
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{1., 2.}, {3., 4.}})),
                        {"1", "2"});
  manager.declareMatrix("test",
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{-1., 4.}, {-10., -2.}})),
                        {"3"});
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
  manager.declareMatrix("a", mfem::DenseMatrix({{2., 1.}, {0., 0.}}));
  manager.declareMatrix("b", 2, matrix_func);
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
  manager.declareMatrix("a", mfem::DenseMatrix({{2., 1.}, {0., 1.}}), {"1", "2"});
  manager.declareMatrix("b", 2, matrix_func, {"-1", "0"});
  manager.declareMatrix("c",
                        _matrix_manager.make<mfem::MatrixConstantCoefficient>(
                            mfem::DenseMatrix({{1., 2.}, {3., 4.}})),
                        {"42", "45"});
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
  manager.declareScalar("a", 2.);
  manager.declareVector("a", mfem::Vector({2., 1.}));
  manager.declareMatrix("a", mfem::DenseMatrix({{2., 1.}, {0., 1.}}));

  manager.declareMatrix("b", mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareScalar("b", 2.);
  manager.declareVector("b", mfem::Vector({2., 1.}));

  manager.declareVector("c", mfem::Vector({2., 1.}));
  manager.declareMatrix("c", mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareScalar("c", 2.);

  // Check that coefficients can not be redefined
  EXPECT_THROW(manager.declareScalar("a", 2.), MooseException);
  EXPECT_THROW(manager.declareVector("a", mfem::Vector({2., 1.})), MooseException);
  EXPECT_THROW(manager.declareMatrix("a", mfem::DenseMatrix({{2., 1.}, {0., 1.}})), MooseException);
}
