#include "MFEMObjectUnitTest.h"
#include "MFEMCurlCurlKernel.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMVectorFEDomainLFKernel.h"
#include "MFEMVectorFEMassKernel.h"
#include "MFEMVectorFEWeakDivergenceKernel.h"

class MFEMKernelTest : public MFEMObjectUnitTest
{
public:
  MFEMKernelTest() : MFEMObjectUnitTest("PlatypusApp") {}
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
  kernel_params.set<std::string>("variable") = "test_variable_name";
  kernel_params.set<std::string>("coefficient") = "coef1";
  MFEMCurlCurlKernel & kernel =
      addObject<MFEMCurlCurlKernel>("MFEMCurlCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::CurlCurlIntegrator *>(kernel.createIntegrator());
  ASSERT_NE(integrator, nullptr);
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
  kernel_params.set<std::string>("variable") = "test_variable_name";
  kernel_params.set<std::string>("coefficient") = "coef1";
  MFEMDiffusionKernel & kernel =
      addObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DiffusionIntegrator *>(kernel.createIntegrator());
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
  kernel_params.set<std::string>("variable") = "test_variable_name";
  kernel_params.set<std::string>("coefficient") = "coef1";
  MFEMMixedVectorGradientKernel & kernel = addObject<MFEMMixedVectorGradientKernel>(
      "MFEMMixedVectorGradientKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedVectorGradientIntegrator *>(kernel.createIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEDomainLFKernel creates an mfem::VectorFEDomainLFIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEDomainLFKernel)
{
  // Build required kernel inputs
  InputParameters vec_coef_params = _factory.getValidParams("MFEMVectorConstantCoefficient");
  vec_coef_params.set<double>("value_x") = 1.0;
  vec_coef_params.set<double>("value_y") = 2.0;
  vec_coef_params.set<double>("value_z") = 3.0;
  _mfem_problem->addVectorCoefficient(
      "MFEMVectorConstantCoefficient", "vec_coef1", vec_coef_params);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEDomainLFKernel");
  kernel_params.set<std::string>("variable") = "test_variable_name";
  kernel_params.set<std::string>("vector_coefficient") = "vec_coef1";
  MFEMVectorFEDomainLFKernel & kernel =
      addObject<MFEMVectorFEDomainLFKernel>("MFEMVectorFEDomainLFKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFEDomainLFIntegrator *>(kernel.createIntegrator());
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
  kernel_params.set<std::string>("variable") = "test_variable_name";
  kernel_params.set<std::string>("coefficient") = "coef1";
  MFEMVectorFEMassKernel & kernel =
      addObject<MFEMVectorFEMassKernel>("MFEMVectorFEMassKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFEMassIntegrator *>(kernel.createIntegrator());
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
  kernel_params.set<std::string>("variable") = "test_variable_name";
  kernel_params.set<std::string>("coefficient") = "coef1";
  MFEMVectorFEWeakDivergenceKernel & kernel = addObject<MFEMVectorFEWeakDivergenceKernel>(
      "MFEMVectorFEWeakDivergenceKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::VectorFEWeakDivergenceIntegrator *>(kernel.createIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}
