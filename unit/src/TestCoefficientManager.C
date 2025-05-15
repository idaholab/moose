#ifdef MFEM_ENABLED

#include <algorithm>

#include "gtest/gtest.h"
#include "CoefficientManager.h"

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

class CheckCoefficientManager : public testing::Test
{
protected:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point1, point2;
  Moose::MFEM::CoefficientManager manager;
  CheckCoefficientManager()
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

TEST_F(CheckCoefficientManager, DeclareUniformScalar)
{
  auto & cref = manager.declareScalar<mfem::ConstantCoefficient>("resistivity", 2.);
  mfem::ConstantCoefficient * c =
      dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarCoefficient("resistivity"));
  EXPECT_EQ(&cref, c);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
}

TEST_F(CheckCoefficientManager, DeclarePWScalar)
{
  mfem::Coefficient & cref1 =
      manager.declareScalarProperty<mfem::ConstantCoefficient>("test", {"1", "2"}, 2.);
  mfem::Coefficient & cref2 =
      manager.declareScalarProperty<mfem::ConstantCoefficient>("test", {"3"}, 1.);
  mfem::PWCoefficient * c =
      dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarCoefficient("test"));
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&cref1, c);
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

TEST_F(CheckCoefficientManager, DeclareFunctionScalar)
{
  manager.declareScalar<mfem::FunctionCoefficient>("resistivity", scalar_func);
  auto c = &manager.getScalarCoefficient("resistivity");
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
}

TEST_F(CheckCoefficientManager, DeclareFunctionTScalar)
{
  manager.declareScalar<mfem::FunctionCoefficient>("resistivity", scalar_func_t);
  auto c = &manager.getScalarCoefficient("resistivity");
  c->SetTime(0.);
  EXPECT_EQ(c->Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 1.5);
  c->SetTime(1.);
  EXPECT_EQ(c->Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.5);
}

TEST_F(CheckCoefficientManager, DeclareFunctionPWScalar)
{
  manager.declareScalarProperty<mfem::FunctionCoefficient>("test", {"1", "2"}, scalar_func);
  manager.declareScalarProperty<mfem::FunctionCoefficient>(
      "test", {"3"}, [](const mfem::Vector & x) -> mfem::real_t { return scalar_func(x) + 1.; });
  mfem::PWCoefficient * c =
      dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarCoefficient("test"));
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

TEST_F(CheckCoefficientManager, DeclareFunctionTPWScalar)
{
  manager.declareScalarProperty<mfem::FunctionCoefficient>("test", {"1", "2"}, scalar_func_t);
  manager.declareScalarProperty<mfem::FunctionCoefficient>(
      "test", {"3"}, [](const mfem::Vector & x) -> mfem::real_t { return scalar_func(x) + 1.; });
  mfem::PWCoefficient * c =
      dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarCoefficient("test"));
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

TEST_F(CheckCoefficientManager, DeclareCoefficientScalar)
{
  mfem::Coefficient & cref1 = manager.declareScalar<mfem::ConstantCoefficient>("resistivity", 2.);
  mfem::Coefficient & cref2 = manager.declareScalar<mfem::ConstantCoefficient>("permittivity", 3.);
  mfem::ConstantCoefficient * c =
      dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarCoefficient("resistivity"));
  EXPECT_EQ(&cref1, c);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
  c = dynamic_cast<mfem::ConstantCoefficient *>(&manager.getScalarCoefficient("permittivity"));
  EXPECT_EQ(&cref2, c);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 3.0);
}

TEST_F(CheckCoefficientManager, DeclareCoefficientAliasScalar)
{
  manager.declareScalar<mfem::ConstantCoefficient>("resistivity", 2.);
  mfem::Coefficient & cref = manager.declareScalar("resistivity2", "resistivity");
  mfem::Coefficient &c1 = manager.getScalarCoefficient("resistivity"),
                    &c2 = manager.getScalarCoefficient("resistivity2");
  EXPECT_EQ(&c1, &c2);
  EXPECT_EQ(&cref, &c1);
}

TEST_F(CheckCoefficientManager, DeclarePropertyAliasScalar)
{
  manager.declareScalarProperty<mfem::ConstantCoefficient>("resistivity", {"1", "2"}, 2.);
  manager.declareScalar("resistivity2", "resistivity");
  mfem::Coefficient &c1 = manager.getScalarCoefficient("resistivity"),
                    &c2 = manager.getScalarCoefficient("resistivity2");
  EXPECT_EQ(&c1, &c2);
}

TEST_F(CheckCoefficientManager, DeclarePropertyFromCoefficientNameScalar)
{
  manager.declareScalar<mfem::ConstantCoefficient>("resistivity", 2.);
  mfem::Coefficient & cref =
      manager.declareScalarProperty("resistivity2", {"1", "2"}, "resistivity");
  mfem::Coefficient & c = manager.getScalarCoefficient("resistivity2");
  fe_transform.Attribute = 1;
  EXPECT_EQ(&cref, &c);
  EXPECT_EQ(c.Eval(fe_transform, point1), 2.0);
  fe_transform.Attribute = 10;
  EXPECT_EQ(c.Eval(fe_transform, point1), 0.0);
}

TEST_F(CheckCoefficientManager, DeclarePropertyFromPropertyNameScalar)
{
  manager.declareScalarProperty<mfem::ConstantCoefficient>("test", {"1", "2"}, 2.);
  manager.declareScalarProperty<mfem::ConstantCoefficient>("test2", {"1", "2"}, 3.);
  EXPECT_THROW(manager.declareScalarProperty("test2", {"3"}, "test"), MooseException);
}

TEST_F(CheckCoefficientManager, NonexistentAliasScalar)
{
  EXPECT_THROW(manager.declareScalar("thingy", "undeclared name"), MooseException);
  EXPECT_THROW(manager.declareScalarProperty("test2", {"1", "2", "3"}, "another undeclared name"),
               MooseException);
}

TEST_F(CheckCoefficientManager, DeclareCoefficientPWScalar)
{
  mfem::Coefficient & cref1 =
      manager.declareScalarProperty<mfem::ConstantCoefficient>("test", {"1", "2"}, 2.);
  mfem::Coefficient & cref2 =
      manager.declareScalarProperty<mfem::ConstantCoefficient>("test", {"3"}, 1.);
  mfem::PWCoefficient * c =
      dynamic_cast<mfem::PWCoefficient *>(&manager.getScalarCoefficient("test"));
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&cref1, c);
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

TEST_F(CheckCoefficientManager, ScalarIsDefined)
{
  manager.declareScalar<mfem::ConstantCoefficient>("a", 2.);
  manager.declareScalar<mfem::FunctionCoefficient>("b", scalar_func);
  manager.declareScalar<mfem::ConstantCoefficient>("c", 2.);
  EXPECT_TRUE(manager.scalarPropertyIsDefined("a", "1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("a", "10"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("A", "1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("b", "1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("b", "-57"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("B", "1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("c", "0"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("c", "20"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("C", "0"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "0"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "2"));

  EXPECT_FALSE(manager.vectorPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("c", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("c", "1"));
}

TEST_F(CheckCoefficientManager, ScalarPWIsDefined)
{
  manager.declareScalarProperty<mfem::ConstantCoefficient>("a", {"1", "2"}, 2.);
  manager.declareScalarProperty<mfem::FunctionCoefficient>("b", {"-1", "0"}, scalar_func);
  manager.declareScalarProperty<mfem::ConstantCoefficient>("c", {"42", "45"}, 2.);
  EXPECT_TRUE(manager.scalarPropertyIsDefined("a", "1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("a", "2"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("a", "0"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("A", "1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("b", "-1"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("b", "0"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("B", "0"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("c", "42"));
  EXPECT_TRUE(manager.scalarPropertyIsDefined("c", "45"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("c", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("C", "42"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "-1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "0"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("d", "2"));

  EXPECT_FALSE(manager.vectorPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("b", "-1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("b", "-1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("c", "42"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("c", "42"));
}

TEST_F(CheckCoefficientManager, DeclareUniformVector)
{

  auto & cref =
      manager.declareVector<mfem::VectorConstantCoefficient>("resistivity", mfem::Vector({1., 2.}));
  mfem::VectorConstantCoefficient * c =
      dynamic_cast<mfem::VectorConstantCoefficient *>(&manager.getVectorCoefficient("resistivity"));
  EXPECT_EQ(&cref, c);
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
}

TEST_F(CheckCoefficientManager, DeclarePWVector)
{
  mfem::VectorCoefficient & cref1 = manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "test", {"1", "2"}, mfem::Vector({1., 2.}));
  mfem::VectorCoefficient & cref2 = manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "test", {"3"}, mfem::Vector({3., 4.}));
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorCoefficient("test"));
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&cref1, c);
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

TEST_F(CheckCoefficientManager, DeclareFunctionVector)
{
  manager.declareVector<mfem::VectorFunctionCoefficient>("resistivity", 2, vector_func);
  auto c = &manager.getVectorCoefficient("resistivity");
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 1.);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.);
}

TEST_F(CheckCoefficientManager, DeclareFunctionTVector)
{
  manager.declareVector<mfem::VectorFunctionCoefficient>("resistivity", 2, vector_func_t);
  auto c = &manager.getVectorCoefficient("resistivity");
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

TEST_F(CheckCoefficientManager, DeclareFunctionPWVector)
{
  manager.declareVectorProperty<mfem::VectorFunctionCoefficient>(
      "test", {"1", "2"}, 2, vector_func);
  manager.declareVectorProperty<mfem::VectorFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::Vector & vec)
      {
        vector_func(x, vec);
        vec *= scalar_func(x);
      });
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorCoefficient("test"));
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
TEST_F(CheckCoefficientManager, DeclareFunctionTPWVector)
{
  manager.declareVectorProperty<mfem::VectorFunctionCoefficient>(
      "test", {"1", "2"}, 2, vector_func_t);
  manager.declareVectorProperty<mfem::VectorFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::Vector & vec)
      {
        vector_func(x, vec);
        vec *= scalar_func(x);
      });
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorCoefficient("test"));
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

TEST_F(CheckCoefficientManager, DeclareCoefficientVector)
{
  mfem::VectorCoefficient & cref1 =
      manager.declareVector<mfem::VectorConstantCoefficient>("resistivity", mfem::Vector({1., 2.}));
  mfem::VectorCoefficient & cref2 = manager.declareVector<mfem::VectorConstantCoefficient>(
      "permittivity", mfem::Vector({3., 4.}));
  mfem::VectorConstantCoefficient * c =
      dynamic_cast<mfem::VectorConstantCoefficient *>(&manager.getVectorCoefficient("resistivity"));
  EXPECT_EQ(&cref1, c);
  ASSERT_NE(c, nullptr);
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.0);
  EXPECT_EQ(vec[1], 2.0);
  c->Eval(vec, fe_transform, point2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.0);
  c = dynamic_cast<mfem::VectorConstantCoefficient *>(
      &manager.getVectorCoefficient("permittivity"));
  EXPECT_EQ(&cref2, c);
  ASSERT_NE(c, nullptr);
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 3.0);
  EXPECT_EQ(vec[1], 4.0);
}

TEST_F(CheckCoefficientManager, DeclareCoefficientAliasVector)
{
  mfem::VectorCoefficient & cref1 =
      manager.declareVector<mfem::VectorConstantCoefficient>("resistivity", mfem::Vector({2., 1.}));
  mfem::VectorCoefficient & cref2 = manager.declareVector("resistivity2", "resistivity");
  mfem::VectorCoefficient &c1 = manager.getVectorCoefficient("resistivity"),
                          &c2 = manager.getVectorCoefficient("resistivity2");
  EXPECT_EQ(&c1, &c2);
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&c1, &cref1);
}

TEST_F(CheckCoefficientManager, DeclarePropertyAliasVector)
{
  manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "resistivity", {"1", "2"}, mfem::Vector({2., 1.}));
  manager.declareVector("resistivity2", "resistivity");
  mfem::VectorCoefficient &c1 = manager.getVectorCoefficient("resistivity"),
                          &c2 = manager.getVectorCoefficient("resistivity2");
  EXPECT_EQ(&c1, &c2);
}

TEST_F(CheckCoefficientManager, DeclarePropertyFromCoefficientNameVector)
{
  manager.declareVector<mfem::VectorConstantCoefficient>("resistivity", mfem::Vector({2., 1.}));
  mfem::VectorCoefficient & cref =
      manager.declareVectorProperty("resistivity2", {"1", "2"}, "resistivity");
  mfem::VectorCoefficient & c = manager.getVectorCoefficient("resistivity2");
  EXPECT_EQ(&cref, &c);
  mfem::Vector vec;
  fe_transform.Attribute = 1;
  c.Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 2.0);
  EXPECT_EQ(vec[1], 1.0);
  fe_transform.Attribute = 10;
  c.Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.0);
  EXPECT_EQ(vec[1], 0.0);
}

TEST_F(CheckCoefficientManager, DeclarePropertyFromPropertyNameVector)
{
  manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "test", {"1", "2"}, mfem::Vector({2., 1.}));
  manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "test2", {"1", "2"}, (mfem::Vector({2., 1.})));
  EXPECT_THROW(manager.declareVectorProperty("test2", {"3"}, "test"), MooseException);
}

TEST_F(CheckCoefficientManager, NonexistentAliasVector)
{
  EXPECT_THROW(manager.declareVector("thingy", "undeclared name"), MooseException);
  EXPECT_THROW(manager.declareVectorProperty("test2", {"1", "2", "3"}, "another undeclared name"),
               MooseException);
}

TEST_F(CheckCoefficientManager, DeclareCoefficientPWVector)
{
  mfem::VectorCoefficient & cref1 = manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "test", {"1", "2"}, mfem::Vector({2., 1.}));
  mfem::VectorCoefficient & cref2 = manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "test", {"3"}, mfem::Vector({-1., -7.}));
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&manager.getVectorCoefficient("test"));
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&cref1, c);
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

TEST_F(CheckCoefficientManager, VectorIsDefined)
{
  manager.declareVector<mfem::VectorConstantCoefficient>("a", mfem::Vector({2., 1.}));
  manager.declareVector<mfem::VectorFunctionCoefficient>("b", 2, vector_func);
  manager.declareVector<mfem::VectorConstantCoefficient>("c", mfem::Vector({3., 4.}));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("a", "1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("a", "10"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("A", "1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("b", "1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("b", "-57"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("B", "1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("c", "0"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("c", "20"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("C", "0"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "0"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("c", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("c", "1"));
}

TEST_F(CheckCoefficientManager, VectorPWIsDefined)
{
  manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "a", {"1", "2"}, mfem::Vector({2., 1.}));
  manager.declareVectorProperty<mfem::VectorFunctionCoefficient>("b", {"-1", "0"}, 2, vector_func);
  manager.declareVectorProperty<mfem::VectorConstantCoefficient>(
      "c", {"42", "45"}, mfem::Vector({3., 4.}));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("a", "1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("a", "2"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("a", "0"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("A", "1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("b", "-1"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("b", "0"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("B", "0"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("c", "42"));
  EXPECT_TRUE(manager.vectorPropertyIsDefined("c", "45"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("c", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("C", "42"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "-1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "0"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("b", "-1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("b", "-1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("c", "42"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("c", "42"));
}

TEST_F(CheckCoefficientManager, DeclareUniformMatrix)
{
  auto & cref = manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "resistivity", mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  mfem::MatrixConstantCoefficient * c =
      dynamic_cast<mfem::MatrixConstantCoefficient *>(&manager.getMatrixCoefficient("resistivity"));
  EXPECT_EQ(&cref, c);
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

TEST_F(CheckCoefficientManager, DeclarePWMatrix)
{
  mfem::MatrixCoefficient & cref1 = manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "test", {"1", "2"}, mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  mfem::MatrixCoefficient & cref2 = manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "test", {"3"}, mfem::DenseMatrix({{-1., 4.}, {-10., -2.}}));
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixCoefficient("test"));
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&cref1, c);
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

TEST_F(CheckCoefficientManager, DeclareFunctionMatrix)
{
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("resistivity", 2, matrix_func);
  auto c = &manager.getMatrixCoefficient("resistivity");
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

TEST_F(CheckCoefficientManager, DeclareFunctionTMatrix)
{
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("resistivity", 2, matrix_func_t);
  auto c = &manager.getMatrixCoefficient("resistivity");
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

TEST_F(CheckCoefficientManager, DeclareCoefficientAliasMatrix)
{
  mfem::MatrixCoefficient & cref1 = manager.declareMatrix<mfem::MatrixConstantCoefficient>(
      "resistivity", mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  mfem::MatrixCoefficient & cref2 = manager.declareMatrix("resistivity2", "resistivity");
  mfem::MatrixCoefficient &c1 = manager.getMatrixCoefficient("resistivity"),
                          &c2 = manager.getMatrixCoefficient("resistivity2");
  EXPECT_EQ(&cref1, &cref2);
  EXPECT_EQ(&c1, &c2);
  EXPECT_EQ(&cref1, &c1);
}

TEST_F(CheckCoefficientManager, DeclarePropertyAliasMatrix)
{
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "resistivity", {"1", "2"}, mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  manager.declareMatrix("resistivity2", "resistivity");
  mfem::MatrixCoefficient &c1 = manager.getMatrixCoefficient("resistivity"),
                          &c2 = manager.getMatrixCoefficient("resistivity2");
  EXPECT_EQ(&c1, &c2);
}

TEST_F(CheckCoefficientManager, DeclarePropertyFromCoefficientNameMatrix)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("resistivity",
                                                         mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  mfem::MatrixCoefficient & cref =
      manager.declareMatrixProperty("resistivity2", {"1", "2"}, "resistivity");
  mfem::MatrixCoefficient & c = manager.getMatrixCoefficient("resistivity2");
  EXPECT_EQ(&cref, &c);
  mfem::DenseMatrix mat;
  fe_transform.Attribute = 1;
  c.Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 1.);
  EXPECT_EQ(mat.Elem(0, 1), 2.);
  EXPECT_EQ(mat.Elem(1, 0), 3.);
  EXPECT_EQ(mat.Elem(1, 1), 4.);
  fe_transform.Attribute = 10;
  c.Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 0.);
  EXPECT_EQ(mat.Elem(0, 1), 0.);
  EXPECT_EQ(mat.Elem(1, 0), 0.);
  EXPECT_EQ(mat.Elem(1, 1), 0.);
}

TEST_F(CheckCoefficientManager, DeclarePropertyFromPropertyNameMatrix)
{
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "test", {"1", "2"}, mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "test2", {"1", "2"}, mfem::DenseMatrix({{2., 2.}, {3., 3.}}));
  EXPECT_THROW(manager.declareMatrixProperty("test2", {"3"}, "test"), MooseException);
}

TEST_F(CheckCoefficientManager, NonexistentAliasMatrix)
{
  EXPECT_THROW(manager.declareMatrix("thingy", "undeclared name"), MooseException);
  EXPECT_THROW(manager.declareMatrixProperty("test2", {"1", "2", "3"}, "another undeclared name"),
               MooseException);
}

TEST_F(CheckCoefficientManager, DeclareFunctionPWMatrix)
{
  manager.declareMatrixProperty<mfem::MatrixFunctionCoefficient>(
      "test", {"1", "2"}, 2, matrix_func);
  manager.declareMatrixProperty<mfem::MatrixFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::DenseMatrix & mat)
      {
        matrix_func(x, mat);
        mat *= scalar_func(x);
      });
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixCoefficient("test"));
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

TEST_F(CheckCoefficientManager, DeclareFunctionTPWMatrix)
{
  manager.declareMatrixProperty<mfem::MatrixFunctionCoefficient>(
      "test", {"1", "2"}, 2, matrix_func_t);
  manager.declareMatrixProperty<mfem::MatrixFunctionCoefficient>(
      "test",
      {"3"},
      2,
      [](const mfem::Vector & x, mfem::DenseMatrix & mat)
      {
        matrix_func(x, mat);
        mat *= scalar_func(x);
      });
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixCoefficient("test"));
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

TEST_F(CheckCoefficientManager, DeclareCoefficientMatrix)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("resistivity",

                                                         mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("permittivity",

                                                         mfem::DenseMatrix({{5., 6.}, {7., 8.}}));
  mfem::MatrixConstantCoefficient * c =
      dynamic_cast<mfem::MatrixConstantCoefficient *>(&manager.getMatrixCoefficient("resistivity"));
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
  c = dynamic_cast<mfem::MatrixConstantCoefficient *>(
      &manager.getMatrixCoefficient("permittivity"));
  ASSERT_NE(c, nullptr);
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat.Elem(0, 0), 5.0);
  EXPECT_EQ(mat.Elem(0, 1), 6.0);
  EXPECT_EQ(mat.Elem(1, 0), 7.0);
  EXPECT_EQ(mat.Elem(1, 1), 8.0);
}

TEST_F(CheckCoefficientManager, DeclareCoefficientPWMatrix)
{
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "test",
      {"1", "2"},

      mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "test",
      {"3"},

      mfem::DenseMatrix({{-1., 4.}, {-10., -2.}}));
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&manager.getMatrixCoefficient("test"));
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

TEST_F(CheckCoefficientManager, MatrixIsDefined)
{
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("a",
                                                         mfem::DenseMatrix({{2., 1.}, {0., 0.}}));
  manager.declareMatrix<mfem::MatrixFunctionCoefficient>("b", 2, matrix_func);
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("c",

                                                         mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("a", "1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("a", "10"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("A", "1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("b", "1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("b", "-57"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("B", "1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("c", "0"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("c", "20"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("C", "0"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "0"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("c", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("c", "1"));
}

TEST_F(CheckCoefficientManager, MatrixPWIsDefined)
{
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "a", {"1", "2"}, mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareMatrixProperty<mfem::MatrixFunctionCoefficient>("b", {"-1", "0"}, 2, matrix_func);
  manager.declareMatrixProperty<mfem::MatrixConstantCoefficient>(
      "c",
      {"42", "45"},

      mfem::DenseMatrix({{1., 2.}, {3., 4.}}));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("a", "1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("a", "2"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("a", "0"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("A", "1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("b", "-1"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("b", "0"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("b", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("B", "0"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("c", "42"));
  EXPECT_TRUE(manager.matrixPropertyIsDefined("c", "45"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("c", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("C", "42"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "-1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "0"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "1"));
  EXPECT_FALSE(manager.matrixPropertyIsDefined("d", "2"));

  EXPECT_FALSE(manager.scalarPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("a", "1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("b", "-1"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("b", "-1"));
  EXPECT_FALSE(manager.scalarPropertyIsDefined("c", "42"));
  EXPECT_FALSE(manager.vectorPropertyIsDefined("c", "42"));
}

TEST_F(CheckCoefficientManager, CheckRepeatedNames)
{
  // Check there can be scalar, vector, and matrix coefficients defined with the same name
  manager.declareScalar<mfem::ConstantCoefficient>("a", 2.);
  manager.declareVector<mfem::VectorConstantCoefficient>("a", mfem::Vector({2., 1.}));
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("a",
                                                         mfem::DenseMatrix({{2., 1.}, {0., 1.}}));

  manager.declareMatrix<mfem::MatrixConstantCoefficient>("b",
                                                         mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareScalar<mfem::ConstantCoefficient>("b", 2.);
  manager.declareVector<mfem::VectorConstantCoefficient>("b", mfem::Vector({2., 1.}));

  manager.declareVector<mfem::VectorConstantCoefficient>("c", mfem::Vector({2., 1.}));
  manager.declareMatrix<mfem::MatrixConstantCoefficient>("c",
                                                         mfem::DenseMatrix({{2., 1.}, {0., 1.}}));
  manager.declareScalar<mfem::ConstantCoefficient>("c", 2.);

  // Check that coefficients can not be redefined
  EXPECT_THROW(manager.declareScalar<mfem::ConstantCoefficient>("a", 2.), MooseException);
  EXPECT_THROW(manager.declareVector<mfem::VectorConstantCoefficient>("a", mfem::Vector({2., 1.})),
               MooseException);
  EXPECT_THROW(manager.declareMatrix<mfem::MatrixConstantCoefficient>(
                   "a", mfem::DenseMatrix({{2., 1.}, {0., 1.}})),
               MooseException);
}

#endif
