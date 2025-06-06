#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMScalarFunctorBoundaryIntegratedBC.h"
#include "MFEMVectorBoundaryIntegratedBC.h"
#include "MFEMVectorNormalIntegratedBC.h"
#include "MFEMVectorFunctorBoundaryIntegratedBC.h"
#include "MFEMVectorFunctorNormalIntegratedBC.h"
#include "MFEMConvectiveHeatFluxBC.h"

class MFEMIntegratedBCTest : public MFEMObjectUnitTest
{
public:
  MFEMIntegratedBCTest() : MFEMObjectUnitTest("MooseUnitApp") {}
};

/**
 * Test MFEMVectorNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMVectorNormalIntegratedBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorNormalIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMVectorNormalIntegratedBC & integrated_bc =
      addObject<MFEMVectorNormalIntegratedBC>("MFEMVectorNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMVectorNormalIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_TRUE(lf_integrator != nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_TRUE(blf_integrator == nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMVectorNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMVectorFunctorNormalIntegratedBC)
{
  // Construct boundary condition
  InputParameters func_params = _factory.getValidParams("ParsedVectorFunction");
  func_params.set<std::string>("expression_x") = "x + y";
  func_params.set<std::string>("expression_y") = "x + y + 1";
  func_params.set<std::string>("expression_z") = "x + y + 2";
  _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params);
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctorNormalIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMVectorFunctorNormalIntegratedBC & integrated_bc =
      addObject<MFEMVectorFunctorNormalIntegratedBC>(
          "MFEMVectorFunctorNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMVectorNormalIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_TRUE(lf_integrator != nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_TRUE(blf_integrator == nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMScalarFunctorBoundaryIntegratedBC creates the expected mfem::BoundaryIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMScalarFunctorBoundaryIntegratedBC)
{
  // Build required BC inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericConstantFunctorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<double>>("prop_values") = {3.0};
  _mfem_problem->addFunctorMaterial("MFEMGenericConstantFunctorMaterial", "material1", coef_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMScalarFunctorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMScalarFunctorBoundaryIntegratedBC & integrated_bc =
      addObject<MFEMScalarFunctorBoundaryIntegratedBC>(
          "MFEMScalarFunctorBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMScalarFunctorBoundaryIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMConvectiveHeatFluxBC creates the expected mfem::BoundaryIntegrators successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMConvectiveHeatFluxBC)
{
  // Build required BC inputs
  InputParameters htc_params = _factory.getValidParams("ParsedFunction");
  htc_params.set<std::string>("expression") = "1.0";
  _mfem_problem->addFunction("ParsedFunction", "htc", htc_params);
  _mfem_problem->getFunction("htc").initialSetup();
  InputParameters Tinf_params = _factory.getValidParams("ParsedFunction");
  Tinf_params.set<std::string>("expression") = "3.0";
  _mfem_problem->addFunction("ParsedFunction", "Tinf", Tinf_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMConvectiveHeatFluxBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("heat_transfer_coefficient") = "htc";
  bc_params.set<MFEMScalarCoefficientName>("T_infinity") = "Tinf";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMConvectiveHeatFluxBC & integrated_bc =
      addObject<MFEMConvectiveHeatFluxBC>("MFEMConvectiveHeatFluxBC", "bc1", bc_params);

  // Test MFEMConvectiveHeatFluxBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator =
      dynamic_cast<mfem::BoundaryMassIntegrator *>(integrated_bc.createBFIntegrator());
  ASSERT_NE(blf_integrator, nullptr);
  delete blf_integrator;
}

TEST_F(MFEMIntegratedBCTest, MFEMVectorBoundaryIntegratedBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  auto & bc =
      addObject<MFEMVectorBoundaryIntegratedBC>("MFEMVectorBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMVectorBoundaryIntegratedBC returns an integrator of the expected type
  auto lf_integrator = dynamic_cast<mfem::VectorBoundaryLFIntegrator *>(bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = bc.createBFIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

TEST_F(MFEMIntegratedBCTest, MFEMVectorFunctorBoundaryIntegratedBC)
{
  // Build required BC inputs
  InputParameters func_params = _factory.getValidParams("ParsedVectorFunction");
  func_params.set<std::string>("expression_x") = "x + y";
  func_params.set<std::string>("expression_y") = "x + y + 1";
  func_params.set<std::string>("expression_z") = "x + y + 2";
  _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func1";
  auto & bc = addObject<MFEMVectorFunctorBoundaryIntegratedBC>(
      "MFEMVectorFunctorBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMVectorBoundaryIntegratedBC returns an integrator of the expected type
  auto lf_integrator = dynamic_cast<mfem::VectorBoundaryLFIntegrator *>(bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = bc.createBFIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

#endif
