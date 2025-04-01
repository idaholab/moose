#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMCurlCurlKernel.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMDivDivKernel.h"
#include "MFEMLinearElasticityKernel.h"
#include "MFEMMixedScalarCurlKernel.h"
#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMVectorDomainLFKernel.h"
#include "MFEMVectorFEDomainLFKernel.h"
#include "MFEMVectorFEMassKernel.h"
#include "MFEMVectorFEWeakDivergenceKernel.h"

class MFEMKernelTest : public MFEMObjectUnitTest
{
public:
  MFEMKernelTest() : MFEMObjectUnitTest("MooseUnitApp") {}
};

/**
 * Test MFEMCurlCurlKernel creates an mfem::CurlCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMCurlCurlKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMCurlCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  MFEMCurlCurlKernel & kernel =
      addObject<MFEMCurlCurlKernel>("MFEMCurlCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::CurlCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_TRUE(integrator != nullptr);
  delete integrator;
}

/**
 * Test MFEMDiffusionKernel creates an mfem::DiffusionIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDiffusionKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<double>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDiffusionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  MFEMDiffusionKernel & kernel =
      addObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DiffusionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMDiffusionKernel creates an mfem::DiffusionIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDivDivKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<double>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDivDivKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  auto & kernel = addObject<MFEMDivDivKernel>("MFEMDivDivKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DivDivIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMLinearElasticityKernel creates an mfem::ElasticityIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMLinearElasticityKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"lambda", "mu"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0, 3.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMLinearElasticityKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("lambda") = "lambda";
  kernel_params.set<MFEMScalarCoefficientName>("mu") = "mu";
  MFEMLinearElasticityKernel & kernel =
      addObject<MFEMLinearElasticityKernel>("MFEMLinearElasticityKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::ElasticityIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorGradientKernel creates an mfem::MixedVectorGradientIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorGradientKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<double>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorGradientKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  MFEMMixedVectorGradientKernel & kernel = addObject<MFEMMixedVectorGradientKernel>(
      "MFEMMixedVectorGradientKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedVectorGradientIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorDomainLFKernel creates an mfem::VectorDomainLFIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorDomainLFKernel)
{
  mfem::Vector expected1({2.0, 1.0, 0.0});
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantVectorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<Real>>("prop_values") = {expected1[0], expected1[1], expected1[2]};
  _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorDomainLFKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "coef1";
  MFEMVectorDomainLFKernel & kernel =
      addObject<MFEMVectorDomainLFKernel>("MFEMVectorDomainLFKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorDomainLFIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEDomainLFKernel creates an mfem::VectorFEDomainLFIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEDomainLFKernel)
{
  // Build required kernel inputs
  InputParameters func_params1 = _factory.getValidParams("ParsedVectorFunction");
  func_params1.set<std::string>("expression_x") = "1.";
  func_params1.set<std::string>("expression_y") = "2.";
  func_params1.set<std::string>("expression_z") = "3.";
  _mfem_problem->addFunction("ParsedVectorFunction", "vec_coef1", func_params1);
  _mfem_problem->getFunction("vec_coef1").initialSetup();

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEDomainLFKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "vec_coef1";
  MFEMVectorFEDomainLFKernel & kernel =
      addObject<MFEMVectorFEDomainLFKernel>("MFEMVectorFEDomainLFKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFEDomainLFIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEMassKernel creates an mfem::VectorFEMassIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEMassKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<double>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEMassKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  MFEMVectorFEMassKernel & kernel =
      addObject<MFEMVectorFEMassKernel>("MFEMVectorFEMassKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFEMassIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEWeakDivergenceKernel creates an mfem::VectorFEWeakDivergenceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEWeakDivergenceKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<double>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEWeakDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  MFEMVectorFEWeakDivergenceKernel & kernel = addObject<MFEMVectorFEWeakDivergenceKernel>(
      "MFEMVectorFEWeakDivergenceKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::VectorFEWeakDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarCurlKernel creates an mfem::MixedScalarCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarCurlKernel)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<Real>>("prop_values") = {2.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  MFEMMixedScalarCurlKernel & kernel =
      addObject<MFEMMixedScalarCurlKernel>("MFEMMixedScalarCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedScalarCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

#endif
