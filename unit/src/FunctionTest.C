#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"

class FunctionTest : public MFEMObjectUnitTest
{
public:
  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point;
  FunctionTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    point.Init(3);
    point.Set3(0., 0., 0.);
    fe_transform.SetIdentityTransformation(mfem::Geometry::CUBE);
  }
};

/**
 * Test MFEMProblem::getScalarFunctionCoefficient works as it should.
 */
TEST_F(FunctionTest, GetScalarFunctionCoefficient)
{
  // Build required kernel inputs
  InputParameters func_params1 = _factory.getValidParams("ParsedFunction");
  func_params1.set<std::string>("expression") = "1.";
  _mfem_problem->addFunction("ParsedFunction", "coef1", func_params1);
  _mfem_problem->getFunction("coef1").initialSetup();
  mfem::Coefficient & coef(_mfem_problem->getProperties().getScalarProperty("coef1"));
  EXPECT_EQ(coef.Eval(fe_transform, point), 1);

  EXPECT_THROW(_mfem_problem->getProperties().getVectorProperty("coef1"), MooseException);
  EXPECT_THROW(_mfem_problem->getProperties().getScalarProperty("coef2"), MooseException);
}

/**
 * Test MFEMProblem::getVectorFunctionCoefficient works as it should.
 */
TEST_F(FunctionTest, GetVectorFunctionCoefficient)
{
  // Build required kernel inputs
  InputParameters func_params1 = _factory.getValidParams("ParsedVectorFunction");
  func_params1.set<std::string>("expression_x") = "1.";
  func_params1.set<std::string>("expression_y") = "2.";
  func_params1.set<std::string>("expression_z") = "3.";
  _mfem_problem->addFunction("ParsedVectorFunction", "vec_coef1", func_params1);
  _mfem_problem->getFunction("vec_coef1").initialSetup();
  mfem::VectorCoefficient & coef(_mfem_problem->getProperties().getVectorProperty("vec_coef1"));
  mfem::Vector vec;
  coef.Eval(vec, fe_transform, point);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.);
  EXPECT_EQ(vec[2], 3.);

  EXPECT_THROW(_mfem_problem->getProperties().getVectorProperty("vec_coef2"), MooseException);
  EXPECT_THROW(_mfem_problem->getProperties().getScalarProperty("vec_coef1"), MooseException);
}

/**
 * Test MFEMProblem::getVectorFunctionCoefficient works as it should with 2D vector functions.
 */
TEST_F(FunctionTest, GetVectorFunctionCoefficient2D)
{
  // Build required kernel inputs
  InputParameters func_params1 = _factory.getValidParams("ParsedVectorFunction");
  func_params1.set<std::string>("expression_x") = "1.";
  func_params1.set<std::string>("expression_y") = "2.";
  _mfem_problem->addFunction("ParsedVectorFunction", "vec_coef1", func_params1);
  _mfem_problem->getFunction("vec_coef1").initialSetup();
  std::shared_ptr<mfem::VectorCoefficient> coef =
      _mfem_problem->getVectorFunctionCoefficient("vec_coef1");
  mfem::Vector vec;
  coef->Eval(vec, fe_transform, point);
  EXPECT_EQ(vec.Size(), 2);
  EXPECT_EQ(vec[0], 1.);
  EXPECT_EQ(vec[1], 2.);
}

/**
 * Test MFEMProblem::getVectorFunctionCoefficient works as it should with 1D vector functions.
 */
TEST_F(FunctionTest, GetVectorFunctionCoefficient1D)
{
  // Build required kernel inputs
  InputParameters func_params1 = _factory.getValidParams("ParsedVectorFunction");
  func_params1.set<std::string>("expression_x") = "1.";
  _mfem_problem->addFunction("ParsedVectorFunction", "vec_coef1", func_params1);
  _mfem_problem->getFunction("vec_coef1").initialSetup();
  std::shared_ptr<mfem::VectorCoefficient> coef =
      _mfem_problem->getVectorFunctionCoefficient("vec_coef1");
  mfem::Vector vec;
  coef->Eval(vec, fe_transform, point);
  EXPECT_EQ(vec.Size(), 1);
  EXPECT_EQ(vec[0], 1.);
}

/**
 * Test MFEMProblem::addFunction when unkown function type is used.
 */
TEST_F(FunctionTest, AddUnknownFunction)
{
  InputParameters func_params1 = _factory.getValidParams("ParsedFunction");
  func_params1.set<std::string>("expression") = "1.";
  _mfem_problem->addFunction("ParsedFunction", "coef1", func_params1);
  InputParameters func_params2 = _factory.getValidParams("ParsedFunction");
  func_params2.set<std::string>("expression") = "x*y";
  _mfem_problem->addFunction("ParsedFunction", "coef2", func_params2);
  InputParameters func_params3 = _factory.getValidParams("LinearCombinationFunction");
  func_params3.set<std::vector<FunctionName>>("functions") = {"coef1", "coef2"};
  func_params3.set<std::vector<double>>("w") = {1., 2.};
  EXPECT_THROW(_mfem_problem->addFunction("LinearCombinationFunction", "coef3", func_params3),
               std::runtime_error);
  _mfem_problem->getFunction("coef1").initialSetup();
  _mfem_problem->getFunction("coef2").initialSetup();
  _mfem_problem->getFunction("coef3").initialSetup();

  EXPECT_THROW(_mfem_problem->getProperties().getScalarProperty("coef3"), MooseException);
  EXPECT_THROW(_mfem_problem->getProperties().getVectorProperty("coef3"), MooseException);
}

#endif
