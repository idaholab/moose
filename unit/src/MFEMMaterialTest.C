#include "MFEMObjectUnitTest.h"
#include "MFEMGenericConstantMaterial.h"
#include "MFEMGenericConstantVectorMaterial.h"
#include "MFEMGenericFunctionMaterial.h"
#include "MFEMGenericFunctionVectorMaterial.h"

class MFEMMaterialTest : public MFEMObjectUnitTest
{
public:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point1, point2;
  MFEMMaterialTest() : MFEMObjectUnitTest("PlatypusApp")
  {
    point1.Init(2);
    point1.Set2(0., 0.);
    point2.Init(2);
    point2.Set2(0.5, 1.);
    fe_transform.SetIdentityTransformation(mfem::Geometry::SQUARE);
  }
};

/**
 * Test whether an MFEMConstantMaterial creates ConstantCoefficients.
 */
TEST_F(MFEMMaterialTest, MFEMGenericConstantMaterial)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0, 1.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  mfem::Coefficient & coef1 = _mfem_problem->getProperties().getScalarProperty("coef1");
  auto c1 = dynamic_cast<mfem::ConstantCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 2.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 2.0);

  mfem::Coefficient & coef2 = _mfem_problem->getProperties().getScalarProperty("coef2");
  auto c2 = dynamic_cast<mfem::ConstantCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 1.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 1.0);
}

/**
 * Test whether an MFEMConstantMaterial can create piecewise coefficients.
 */
TEST_F(MFEMMaterialTest, MFEMGenericConstantMaterial_PW)
{
  InputParameters coef_params1 = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params1.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params1.set<std::vector<Real>>("prop_values") = {2.0, 1.0};
  coef_params1.set<std::vector<SubdomainName>>("block") = {"1", "2"};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params1);
  InputParameters coef_params2 = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params2.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params2.set<std::vector<Real>>("prop_values") = {3.0};
  coef_params2.set<std::vector<SubdomainName>>("block") = {"3"};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material2", coef_params2);

  mfem::Coefficient & coef1 = _mfem_problem->getProperties().getScalarProperty("coef1");
  auto c1 = dynamic_cast<mfem::PWCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 2.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 2.0);
  fe_transform.Attribute = 3;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 3.0);
  fe_transform.Attribute = 4;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);

  mfem::Coefficient & coef2 = _mfem_problem->getProperties().getScalarProperty("coef2");
  auto c2 = dynamic_cast<mfem::PWCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 1.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 1.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 1.0);
  fe_transform.Attribute = 3;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
  fe_transform.Attribute = 4;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
}

/**
 * Test how MFEMGenericConstantMaterial behaves when the wrong number
 * of property names/values are passed.
 */
TEST_F(MFEMMaterialTest, MFEMGenericConstantMaterial_Exception)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0};
  EXPECT_THROW(_mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params),
               std::runtime_error);
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0, 1.0};
  EXPECT_THROW(_mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params),
               std::runtime_error);
}

void
check_vector(mfem::VectorCoefficient & coef,
             mfem::IsoparametricTransformation & fe_transform,
             mfem::IntegrationPoint & point,
             const mfem::Vector & expected)
{
  mfem::Vector vec;
  coef.Eval(vec, fe_transform, point);
  EXPECT_EQ(vec[0], expected[0]);
  EXPECT_EQ(vec[1], expected[1]);
  EXPECT_EQ(vec[2], expected[2]);
}

/**
 * Test whether an MFEMConstantVectorMaterial creates ConstantCoefficients.
 */
TEST_F(MFEMMaterialTest, MFEMGenericConstantVectorMaterial)
{
  mfem::Vector expected1({2.0, 1.0, 0.0}), expected2({5.0, 5.5, 6.0});
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantVectorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<Real>>("prop_values") = {
      expected1[0], expected1[1], expected1[2], expected2[0], expected2[1], expected2[2]};
  _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material1", coef_params);

  mfem::VectorCoefficient & coef1 = _mfem_problem->getProperties().getVectorProperty("coef1");
  auto c1 = dynamic_cast<mfem::VectorConstantCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  check_vector(coef1, fe_transform, point1, expected1);
  check_vector(coef1, fe_transform, point2, expected1);
  fe_transform.Attribute = 2;
  check_vector(coef1, fe_transform, point1, expected1);

  mfem::VectorCoefficient & coef2 = _mfem_problem->getProperties().getVectorProperty("coef2");
  auto c2 = dynamic_cast<mfem::VectorConstantCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  check_vector(coef2, fe_transform, point1, expected2);
  check_vector(coef2, fe_transform, point2, expected2);
  fe_transform.Attribute = 2;
  check_vector(coef2, fe_transform, point2, expected2);
}
