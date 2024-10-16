#include "MFEMObjectUnitTest.h"
#include "MFEMVectorNormalIntegratedBC.h"
#include "MFEMConvectiveHeatFluxBC.h"

class MFEMIntegratedBCTest : public MFEMObjectUnitTest
{
public:
  MFEMIntegratedBCTest() : MFEMObjectUnitTest("PlatypusApp") {}
};

/**
 * Test MFEMVectorNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMVectorNormalIntegratedBC)
{
  // Build required kernel inputs
  InputParameters vec_coef_params = _factory.getValidParams("MFEMGenericConstantVectorMaterial");
  vec_coef_params.set<std::vector<std::string>>("prop_names") = {"vec_coef1"};
  vec_coef_params.set<std::vector<double>>("prop_values") = {1.0, 2.0, 3.0};
  _mfem_problem->addMaterial("MFEMGenericConstantVectorMaterial", "material1", vec_coef_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorNormalIntegratedBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<std::string>("vector_coefficient") = "vec_coef1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMVectorNormalIntegratedBC & integrated_bc =
      addObject<MFEMVectorNormalIntegratedBC>("MFEMVectorNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLinearFormIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBilinearFormIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMConvectiveHeatFluxBC creates the expected mfem::BoundaryIntegrators successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMConvectiveHeatFluxBC)
{
  // Build required kernel inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"htc", "Tinf"};
  coef_params.set<std::vector<double>>("prop_values") = {1.0, 3.0};
  _mfem_problem->addMaterial("MFEMGenericConstantMaterial", "material1", coef_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMConvectiveHeatFluxBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<std::string>("heat_transfer_coefficient") = "htc";
  bc_params.set<std::string>("T_infinity") = "Tinf";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMConvectiveHeatFluxBC & integrated_bc =
      addObject<MFEMConvectiveHeatFluxBC>("MFEMConvectiveHeatFluxBC", "bc1", bc_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(integrated_bc.createLinearFormIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator =
      dynamic_cast<mfem::BoundaryMassIntegrator *>(integrated_bc.createBilinearFormIntegrator());
  ASSERT_NE(blf_integrator, nullptr);
  delete blf_integrator;
}
