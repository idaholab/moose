#include <algorithm>

#include "gtest/gtest.h"
#include "property_map.h"

#include "mfem.hpp"

class CheckPropertyMap : public testing::Test
{
protected:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point1, point2;
  platypus::ScalarCoefficientManager _scalar_manager;
  platypus::VectorCoefficientManager _vector_manager;
  platypus::MatrixCoefficientManager _matrix_manager;
  CheckPropertyMap()
  {
    point1.Init(2);
    point1.Set2(0., 0.);
    point2.Init(2);
    point2.Set2(0.5, 0.1);
    fe_transform.SetIdentityTransformation(mfem::Geometry::SQUARE);
  }
};

TEST_F(CheckPropertyMap, HasCoefficient)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addProperty("test", std::make_unique<mfem::ConstantCoefficient>(1.));
  EXPECT_TRUE(prop_map.hasCoefficient("test"));
  EXPECT_FALSE(prop_map.hasCoefficient("missing"));
}

TEST_F(CheckPropertyMap, GetCoefficient)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addProperty("resistivity", std::make_unique<mfem::ConstantCoefficient>(2.));
  mfem::ConstantCoefficient * c =
      dynamic_cast<mfem::ConstantCoefficient *>(&prop_map.getCoefficient("resistivity"));
  EXPECT_NE(c, nullptr);
  EXPECT_EQ(c->Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(c->Eval(fe_transform, point2), 2.0);
}

TEST_F(CheckPropertyMap, GetPWCoefficient)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addPiecewiseBlocks("test", std::make_shared<mfem::ConstantCoefficient>(2.), {"1", "2"});
  prop_map.addPiecewiseBlocks("test", std::make_shared<mfem::ConstantCoefficient>(1.), {"3"});
  mfem::PWCoefficient * c = dynamic_cast<mfem::PWCoefficient *>(&prop_map.getCoefficient("test"));
  EXPECT_NE(c, nullptr);
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

TEST_F(CheckPropertyMap, HasVecCoefficient)
{
  platypus::VectorMap prop_map(_vector_manager);
  prop_map.addProperty(
      "test", std::make_unique<mfem::VectorConstantCoefficient>(mfem::Vector({0., 1., 0.})));
  EXPECT_TRUE(prop_map.hasCoefficient("test"));
  EXPECT_FALSE(prop_map.hasCoefficient("missing"));
}

TEST_F(CheckPropertyMap, GetVecCoefficient)
{
  platypus::VectorMap prop_map(_vector_manager);
  prop_map.addProperty(
      "resistivity", std::make_unique<mfem::VectorConstantCoefficient>(mfem::Vector({0., 1., 0.})));
  mfem::VectorConstantCoefficient * c =
      dynamic_cast<mfem::VectorConstantCoefficient *>(&prop_map.getCoefficient("resistivity"));
  EXPECT_NE(c, nullptr);
  mfem::Vector vec;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 1.);
  EXPECT_EQ(vec[2], 0.);
}

TEST_F(CheckPropertyMap, GetPWVecCoefficient)
{
  platypus::VectorMap prop_map(_vector_manager);
  prop_map.addPiecewiseBlocks(
      "test",
      std::make_shared<mfem::VectorConstantCoefficient>(mfem::Vector({0., 1., 0.})),
      {"1", "2"});
  prop_map.addPiecewiseBlocks(
      "test", std::make_shared<mfem::VectorConstantCoefficient>(mfem::Vector({1., 0., 0.})), {"3"});
  mfem::PWVectorCoefficient * c =
      dynamic_cast<mfem::PWVectorCoefficient *>(&prop_map.getCoefficient("test"));
  EXPECT_NE(c, nullptr);
  mfem::Vector vec;
  fe_transform.Attribute = 1;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 1.);
  EXPECT_EQ(vec[2], 0.);
  fe_transform.Attribute = 2;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 1.);
  EXPECT_EQ(vec[2], 0.);
  fe_transform.Attribute = 3;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 0.);
  EXPECT_EQ(vec[2], 0.);
  fe_transform.Attribute = 10;
  c->Eval(vec, fe_transform, point1);
  EXPECT_EQ(vec[0], 0.);
  EXPECT_EQ(vec[1], 0.);
  EXPECT_EQ(vec[2], 0.);
}

TEST_F(CheckPropertyMap, HasMatCoefficient)
{
  platypus::MatrixMap prop_map(_matrix_manager);
  prop_map.addProperty(
      "test",
      std::make_unique<mfem::MatrixConstantCoefficient>(mfem::DenseMatrix({{0., 1.}, {1., 0.}})));
  EXPECT_TRUE(prop_map.hasCoefficient("test"));
  EXPECT_FALSE(prop_map.hasCoefficient("missing"));
}

TEST_F(CheckPropertyMap, GetMatCoefficient)
{
  platypus::MatrixMap prop_map(_matrix_manager);
  prop_map.addProperty(
      "resistivity",
      std::make_unique<mfem::MatrixConstantCoefficient>(mfem::DenseMatrix({{0., 1.}, {1., 0.}})));
  mfem::MatrixConstantCoefficient * c =
      dynamic_cast<mfem::MatrixConstantCoefficient *>(&prop_map.getCoefficient("resistivity"));
  EXPECT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat(0, 0), 0.);
  EXPECT_EQ(mat(0, 1), 1.);
  EXPECT_EQ(mat(1, 0), 1.);
  EXPECT_EQ(mat(1, 1), 0.);
}

TEST_F(CheckPropertyMap, GetPWMatCoefficient)
{
  platypus::MatrixMap prop_map(_matrix_manager);
  prop_map.addPiecewiseBlocks(
      "test",
      std::make_shared<mfem::MatrixConstantCoefficient>(mfem::DenseMatrix({{0., 1.}, {1., 0.}})),
      {"1", "2"});
  prop_map.addPiecewiseBlocks(
      "test",
      std::make_shared<mfem::MatrixConstantCoefficient>(mfem::DenseMatrix({{11., 0.}, {0., -1.}})),
      {"3"});
  mfem::PWMatrixCoefficient * c =
      dynamic_cast<mfem::PWMatrixCoefficient *>(&prop_map.getCoefficient("test"));
  EXPECT_NE(c, nullptr);
  mfem::DenseMatrix mat;
  fe_transform.Attribute = 1;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat(0, 0), 0.);
  EXPECT_EQ(mat(0, 1), 1.);
  EXPECT_EQ(mat(1, 0), 1.);
  EXPECT_EQ(mat(1, 1), 0.);
  fe_transform.Attribute = 2;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat(0, 0), 0.);
  EXPECT_EQ(mat(0, 1), 1.);
  EXPECT_EQ(mat(1, 0), 1.);
  EXPECT_EQ(mat(1, 1), 0.);
  fe_transform.Attribute = 3;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat(0, 0), 11.);
  EXPECT_EQ(mat(0, 1), 0.);
  EXPECT_EQ(mat(1, 0), 0.);
  EXPECT_EQ(mat(1, 1), -1.);
  fe_transform.Attribute = 10;
  c->Eval(mat, fe_transform, point1);
  EXPECT_EQ(mat(0, 0), 0.);
  EXPECT_EQ(mat(0, 1), 0.);
  EXPECT_EQ(mat(1, 0), 0.);
  EXPECT_EQ(mat(1, 1), 0.);
}

TEST_F(CheckPropertyMap, CoefficientDefinedOnBlock)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addPiecewiseBlocks("a", std::make_shared<mfem::ConstantCoefficient>(2.), {"1", "2"});
  prop_map.addProperty("b", std::make_unique<mfem::ConstantCoefficient>(5.));
  EXPECT_TRUE(prop_map.coefficientDefinedOnBlock("a", "1"));
  EXPECT_TRUE(prop_map.coefficientDefinedOnBlock("a", "2"));
  EXPECT_FALSE(prop_map.coefficientDefinedOnBlock("a", "3"));
  EXPECT_FALSE(prop_map.coefficientDefinedOnBlock("a", "0"));
  EXPECT_TRUE(prop_map.coefficientDefinedOnBlock("b", "0"));
  EXPECT_TRUE(prop_map.coefficientDefinedOnBlock("b", "1"));
  EXPECT_TRUE(prop_map.coefficientDefinedOnBlock("b", "2"));
  EXPECT_FALSE(prop_map.coefficientDefinedOnBlock("c", "1"));
  EXPECT_FALSE(prop_map.coefficientDefinedOnBlock("A", "1"));
  EXPECT_FALSE(prop_map.coefficientDefinedOnBlock("B", "1"));
}

TEST_F(CheckPropertyMap, OverwriteProperty)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addProperty("resistivity", std::make_unique<mfem::ConstantCoefficient>(2.));
  EXPECT_THROW(prop_map.addProperty("resistivity", std::make_unique<mfem::ConstantCoefficient>(4.)),
               MooseException);
}

TEST_F(CheckPropertyMap, PropertyNotDeclared)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  EXPECT_THROW(prop_map.getCoefficient("NotDeclared"), MooseException);
}

TEST_F(CheckPropertyMap, AddBlocksForUniformProperty)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addProperty("resistivity", std::make_unique<mfem::ConstantCoefficient>(2.));
  EXPECT_THROW(prop_map.addPiecewiseBlocks(
                   "resistivity", std::make_shared<mfem::ConstantCoefficient>(1.), {"1", "2"}),
               MooseException);
}

TEST_F(CheckPropertyMap, OverwriteBlocks)
{
  platypus::ScalarMap prop_map(_scalar_manager);
  prop_map.addPiecewiseBlocks(
      "resistivity", std::make_shared<mfem::ConstantCoefficient>(2.), {"2", "3"});
  EXPECT_THROW(prop_map.addPiecewiseBlocks(
                   "resistivity", std::make_shared<mfem::ConstantCoefficient>(1.), {"1", "2"}),
               MooseException);
}

TEST_F(CheckPropertyMap, DifferentVecSize)
{
  platypus::VectorMap prop_map(_vector_manager);
  prop_map.addPiecewiseBlocks(
      "test",
      std::make_shared<mfem::VectorConstantCoefficient>(mfem::Vector({0., 1., 0.})),
      {"1", "2"});
  EXPECT_THROW(
      prop_map.addPiecewiseBlocks(
          "test", std::make_shared<mfem::VectorConstantCoefficient>(mfem::Vector({1., 0.})), {"3"}),
      MooseException);
}

TEST_F(CheckPropertyMap, DifferentMatSize)
{
  platypus::MatrixMap prop_map(_matrix_manager);
  prop_map.addPiecewiseBlocks(
      "test",
      std::make_shared<mfem::MatrixConstantCoefficient>(mfem::DenseMatrix({{0., 1.}, {1., 0.}})),
      {"1", "2"});
  EXPECT_THROW(prop_map.addPiecewiseBlocks("test",
                                           std::make_shared<mfem::MatrixConstantCoefficient>(
                                               mfem::DenseMatrix({{11., 0., 42.}, {0., -1., 10.}})),
                                           {"3"}),
               MooseException);
}
