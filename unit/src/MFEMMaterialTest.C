#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMGenericConstantMaterial.h"
#include "MFEMGenericConstantVectorMaterial.h"
#include "MFEMGenericFunctorMaterial.h"
#include "MFEMGenericFunctorVectorMaterial.h"

class MFEMMaterialTest : public MFEMObjectUnitTest
{
public:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point1, point2;
  MFEMMaterialTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    point1.Init(3);
    point1.Set3(0., 0., 0.);
    point2.Init(3);
    point2.Set3(0.5, 1., 0.);
    fe_transform.SetIdentityTransformation(mfem::Geometry::CUBE);
  }
};

class MFEMFunctorMaterialTest : public MFEMMaterialTest
{
public:
  MFEMFunctorMaterialTest() : MFEMMaterialTest()
  {
    InputParameters func_params1 = _factory.getValidParams("ParsedFunction");
    func_params1.set<std::string>("expression") = "x + y";
    _mfem_problem->addFunction("ParsedFunction", "func1", func_params1);
    _mfem_problem->getFunction("func1").initialSetup();
    InputParameters func_params2 = _factory.getValidParams("ParsedFunction");
    func_params2.set<std::string>("expression") = "x * y";
    _mfem_problem->addFunction("ParsedFunction", "func2", func_params2);
    _mfem_problem->getFunction("func2").initialSetup();
  }
};

class MFEMFunctorVectorMaterialTest : public MFEMMaterialTest
{
public:
  MFEMFunctorVectorMaterialTest() : MFEMMaterialTest()
  {
    InputParameters func_params1 = _factory.getValidParams("ParsedVectorFunction");
    func_params1.set<std::string>("expression_x") = "x + y";
    func_params1.set<std::string>("expression_y") = "x + y + 1";
    func_params1.set<std::string>("expression_z") = "x + y + 2";
    _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params1);
    _mfem_problem->getFunction("func1").initialSetup();
    InputParameters func_params2 = _factory.getValidParams("ParsedVectorFunction");
    func_params2.set<std::string>("expression_x") = "x * y";
    func_params2.set<std::string>("expression_y") = "2 * x * y";
    func_params2.set<std::string>("expression_z") = "3 * x * y";
    _mfem_problem->addFunction("ParsedVectorFunction", "func2", func_params2);
    _mfem_problem->getFunction("func2").initialSetup();
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

  mfem::Coefficient & coef1 = _mfem_problem->getCoefficients().getScalarCoefficient("coef1");
  auto c1 = dynamic_cast<mfem::ConstantCoefficient *>(&coef1);
  EXPECT_TRUE(c1 != nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 2.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 2.0);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 2.0);

  mfem::Coefficient & coef2 = _mfem_problem->getCoefficients().getScalarCoefficient("coef2");
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

  mfem::Coefficient & coef1 = _mfem_problem->getCoefficients().getScalarCoefficient("coef1");
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

  mfem::Coefficient & coef2 = _mfem_problem->getCoefficients().getScalarCoefficient("coef2");
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

testing::AssertionResult
check_vector(mfem::VectorCoefficient & coef,
             mfem::IsoparametricTransformation & fe_transform,
             mfem::IntegrationPoint & point,
             const mfem::Vector & expected)
{
  mfem::Vector vec;
  coef.Eval(vec, fe_transform, point);
  int errors = 0;
  if (vec[0] != expected[0])
    errors++;
  if (vec[1] != expected[1])
    errors++;
  if (vec[2] != expected[2])
    errors++;
  if (errors > 1)
  {
    return testing::AssertionFailure() << "Vector [" << vec[0] << ", " << vec[1] << ", " << vec[2]
                                       << "] does not match expected value [" << expected[0] << ", "
                                       << expected[1] << ", " << expected[2] << "]";
  }
  else
  {
    return testing::AssertionSuccess();
  }
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

  mfem::VectorCoefficient & coef1 = _mfem_problem->getCoefficients().getVectorCoefficient("coef1");
  auto c1 = dynamic_cast<mfem::VectorConstantCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, expected1));
  EXPECT_TRUE(check_vector(coef1, fe_transform, point2, expected1));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, expected1));

  mfem::VectorCoefficient & coef2 = _mfem_problem->getCoefficients().getVectorCoefficient("coef2");
  auto c2 = dynamic_cast<mfem::VectorConstantCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point1, expected2));
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, expected2));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, expected2));
}

/**
 * Test whether an MFEMConstantVectorMaterial can create piecewise coefficients.
 */
TEST_F(MFEMMaterialTest, MFEMGenericConstantVectorMaterial_PW)
{
  mfem::Vector expected1({2.0, 1.0, 0.0}), expected2({5.0, 5.5, 6.0}), zero({0., 0., 0.});
  InputParameters coef_params1 = _factory.getValidParams("MFEMGenericConstantVectorMaterial");
  coef_params1.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params1.set<std::vector<Real>>("prop_values") = {expected1[0], expected1[1], expected1[2]};
  coef_params1.set<std::vector<SubdomainName>>("block") = {"1"};
  _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material1", coef_params1);
  InputParameters coef_params2 = _factory.getValidParams("MFEMGenericConstantVectorMaterial");
  coef_params2.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params2.set<std::vector<Real>>("prop_values") = {expected2[0], expected2[1], expected2[2]};
  coef_params2.set<std::vector<SubdomainName>>("block") = {"2"};
  _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material2", coef_params2);

  mfem::VectorCoefficient & coef = _mfem_problem->getCoefficients().getVectorCoefficient("coef1");
  auto c = dynamic_cast<mfem::PWVectorCoefficient *>(&coef);
  EXPECT_NE(c, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef, fe_transform, point1, expected1));
  EXPECT_TRUE(check_vector(coef, fe_transform, point2, expected1));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef, fe_transform, point1, expected2));
  EXPECT_TRUE(check_vector(coef, fe_transform, point2, expected2));
  fe_transform.Attribute = 3;
  EXPECT_TRUE(check_vector(coef, fe_transform, point1, zero));
  EXPECT_TRUE(check_vector(coef, fe_transform, point2, zero));
}

/**
 * Test how MFEMGenericConstantVectorMaterial behaves when the wrong number
 * of property names/values are passed.
 */
TEST_F(MFEMMaterialTest, MFEMGenericConstantVectorMaterial_Exception)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantVectorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0, 1.0, 3.0, 4.0};
  EXPECT_THROW(
      _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material1", coef_params),
      std::runtime_error);
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0, 1.0, 0.0, -1.0};
  EXPECT_THROW(
      _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material1", coef_params),
      std::runtime_error);
}

/**
 * Test whether an MFEMGenericFunctorMaterial creates FunctionCoefficients.
 */
TEST_F(MFEMFunctorMaterialTest, MFEMGenericFunctorMaterial)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericFunctorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"func1", "func2"};
  _mfem_problem->addMaterial("MFEMGenericFunctorMaterial", "material1", coef_params);

  mfem::Coefficient & coef1 = _mfem_problem->getCoefficients().getScalarCoefficient("coef1");
  auto c1 = dynamic_cast<mfem::FunctionCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);

  mfem::Coefficient & coef2 = _mfem_problem->getCoefficients().getScalarCoefficient("coef2");
  auto c2 = dynamic_cast<mfem::FunctionCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 0.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 0.5);
}

/**
 * Test whether an MFEMFunctionMaterial can create piecewise coefficients.
 */
TEST_F(MFEMFunctorMaterialTest, MFEMGenericFunctorMaterial_PW)
{
  InputParameters coef_params1 = _factory.getValidParams("MFEMGenericFunctorMaterial");
  coef_params1.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params1.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"func1", "func2"};
  coef_params1.set<std::vector<SubdomainName>>("block") = {"1", "2"};
  _mfem_problem->addMaterial("MFEMGenericFunctorMaterial", "material1", coef_params1);
  InputParameters coef_params2 = _factory.getValidParams("MFEMGenericFunctorMaterial");
  coef_params2.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params2.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"func2"};
  coef_params2.set<std::vector<SubdomainName>>("block") = {"3"};
  _mfem_problem->addMaterial("MFEMGenericFunctorMaterial", "material2", coef_params2);

  mfem::Coefficient & coef1 = _mfem_problem->getCoefficients().getScalarCoefficient("coef1");
  auto c1 = dynamic_cast<mfem::PWCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 1.5);
  fe_transform.Attribute = 3;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 0.5);
  fe_transform.Attribute = 4;
  EXPECT_EQ(coef1.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef1.Eval(fe_transform, point2), 0.0);

  mfem::Coefficient & coef2 = _mfem_problem->getCoefficients().getScalarCoefficient("coef2");
  auto c2 = dynamic_cast<mfem::PWCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 0.5);
  fe_transform.Attribute = 2;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 0.5);
  fe_transform.Attribute = 3;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 0.0);
  fe_transform.Attribute = 4;
  EXPECT_EQ(coef2.Eval(fe_transform, point1), 0.0);
  EXPECT_EQ(coef2.Eval(fe_transform, point2), 0.0);
}

/**
 * Test how MFEMGenericFunctorMaterial behaves when the wrong number
 * of property names/values are passed.
 */
TEST_F(MFEMFunctorMaterialTest, MFEMGenericFunctorMaterial_Exception)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericFunctorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"func1"};
  EXPECT_THROW(_mfem_problem->addMaterial("MFEMGenericFunctorMaterial", "material1", coef_params),
               std::runtime_error);
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"func1", "func2"};
  EXPECT_THROW(_mfem_problem->addMaterial("MFEMGenericFunctorMaterial", "material1", coef_params),
               std::runtime_error);
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"func3"};
  EXPECT_THROW(_mfem_problem->addMaterial("MFEMGenericFunctorMaterial", "material1", coef_params),
               std::runtime_error);
}

/**
 * Test whether an MFEMGenericFunctorVectorMaterial creates FunctionCoefficients.
 */
TEST_F(MFEMFunctorVectorMaterialTest, MFEMGenericFunctorVectorMaterial)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericFunctorVectorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<MFEMVectorCoefficientName>>("prop_values") = {"func1", "func2"};
  _mfem_problem->addMaterial("MFEMGenericFunctorVectorMaterial", "material1", coef_params);
  mfem::Vector a({0., 1., 2.}), b({1.5, 2.5, 3.5}), c({0., 0., 0.}), d({0.5, 1., 1.5});

  mfem::VectorCoefficient & coef1 = _mfem_problem->getCoefficients().getVectorCoefficient("coef1");
  auto c1 = dynamic_cast<mfem::VectorFunctionCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, a));
  EXPECT_TRUE(check_vector(coef1, fe_transform, point2, b));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, a));

  mfem::VectorCoefficient & coef2 = _mfem_problem->getCoefficients().getVectorCoefficient("coef2");
  auto c2 = dynamic_cast<mfem::VectorFunctionCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point1, c));
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, d));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, d));
}

/**
 * Test whether an MFEMFunctionVectorMaterial can create piecewise coefficients.
 */
TEST_F(MFEMFunctorVectorMaterialTest, MFEMGenericFunctorVectorMaterial_PW)
{
  InputParameters coef_params1 = _factory.getValidParams("MFEMGenericFunctorVectorMaterial");
  coef_params1.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params1.set<std::vector<MFEMVectorCoefficientName>>("prop_values") = {"func1", "func2"};
  coef_params1.set<std::vector<SubdomainName>>("block") = {"1", "2"};
  _mfem_problem->addMaterial("MFEMGenericFunctorVectorMaterial", "material1", coef_params1);
  InputParameters coef_params2 = _factory.getValidParams("MFEMGenericFunctorVectorMaterial");
  coef_params2.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params2.set<std::vector<MFEMVectorCoefficientName>>("prop_values") = {"func2"};
  coef_params2.set<std::vector<SubdomainName>>("block") = {"3"};
  _mfem_problem->addMaterial("MFEMGenericFunctorVectorMaterial", "material2", coef_params2);
  mfem::Vector a({0., 1., 2.}), b({1.5, 2.5, 3.5}), c({0., 0., 0.}), d({0.5, 1., 1.5}),
      zero({0., 0., 0.});

  mfem::VectorCoefficient & coef1 = _mfem_problem->getCoefficients().getVectorCoefficient("coef1");
  auto c1 = dynamic_cast<mfem::PWVectorCoefficient *>(&coef1);
  EXPECT_NE(c1, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, a));
  EXPECT_TRUE(check_vector(coef1, fe_transform, point2, b));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, a));
  EXPECT_TRUE(check_vector(coef1, fe_transform, point2, b));
  fe_transform.Attribute = 3;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, c));
  EXPECT_TRUE(check_vector(coef1, fe_transform, point2, d));
  fe_transform.Attribute = 4;
  EXPECT_TRUE(check_vector(coef1, fe_transform, point1, zero));
  EXPECT_TRUE(check_vector(coef1, fe_transform, point2, zero));

  mfem::VectorCoefficient & coef2 = _mfem_problem->getCoefficients().getVectorCoefficient("coef2");
  auto c2 = dynamic_cast<mfem::PWVectorCoefficient *>(&coef2);
  EXPECT_NE(c2, nullptr);
  fe_transform.Attribute = 1;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point1, c));
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, d));
  fe_transform.Attribute = 2;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point1, c));
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, d));
  fe_transform.Attribute = 3;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point1, zero));
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, zero));
  fe_transform.Attribute = 4;
  EXPECT_TRUE(check_vector(coef2, fe_transform, point1, zero));
  EXPECT_TRUE(check_vector(coef2, fe_transform, point2, zero));
}

/**
 * Test how MFEMGenericFunctorVectorMaterial behaves when the wrong number
 * of property names/values are passed.
 */
TEST_F(MFEMFunctorVectorMaterialTest, MFEMGenericFunctorVectorMaterial_Exception)
{
  InputParameters coef_params = _factory.getValidParams("MFEMGenericFunctorVectorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1", "coef2"};
  coef_params.set<std::vector<MFEMVectorCoefficientName>>("prop_values") = {"func1"};
  EXPECT_THROW(
      _mfem_problem->addMaterial("MFEMGenericFunctorVectorMaterial", "material1", coef_params),
      std::runtime_error);
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<MFEMVectorCoefficientName>>("prop_values") = {"func1", "func2"};
  EXPECT_THROW(
      _mfem_problem->addMaterial("MFEMGenericFunctorVectorMaterial", "material1", coef_params),
      std::runtime_error);
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<MFEMVectorCoefficientName>>("prop_values") = {"func3"};
  EXPECT_THROW(
      _mfem_problem->addMaterial("MFEMGenericFunctorVectorMaterial", "material1", coef_params),
      std::runtime_error);
}

#endif
