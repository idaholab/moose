#ifdef MOOSE_MFEM_ENABLED

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMObjectUnitTest.h"
#include "MFEMBoundaryIntegratedBC.h"
#include "MFEMVectorBoundaryIntegratedBC.h"
#include "MFEMBoundaryNormalIntegratedBC.h"
#include "MFEMConvectiveHeatFluxBC.h"

class MFEMIntegratedBCTest : public MFEMObjectUnitTest
{
public:
  MFEMIntegratedBCTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    // Register a dummy (Par)GridFunction for the variable the BCs apply to
    auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
    mfem::common::H1_FESpace fe(pm, 1);
    mfem::GridFunction gf(&fe);
    auto pgf = std::make_shared<mfem::ParGridFunction>(pm, &gf);
    _mfem_problem->getProblemData().gridfunctions.Register("test_variable_name", pgf);
  }
};

/**
 * Test MFEMBoundaryNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMVectorNormalIntegratedConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMBoundaryNormalIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMBoundaryNormalIntegratedBC & integrated_bc =
      addObject<MFEMBoundaryNormalIntegratedBC>("MFEMBoundaryNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMBoundaryNormalIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_TRUE(lf_integrator != nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_TRUE(blf_integrator == nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMBoundaryNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMBoundaryNormalIntegratedBC)
{
  // Construct boundary condition
  InputParameters func_params = _factory.getValidParams("ParsedVectorFunction");
  func_params.set<std::string>("expression_x") = "x + y";
  func_params.set<std::string>("expression_y") = "x + y + 1";
  func_params.set<std::string>("expression_z") = "x + y + 2";
  _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params);
  InputParameters bc_params = _factory.getValidParams("MFEMBoundaryNormalIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMBoundaryNormalIntegratedBC & integrated_bc =
      addObject<MFEMBoundaryNormalIntegratedBC>("MFEMBoundaryNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMBoundaryNormalIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_TRUE(lf_integrator != nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_TRUE(blf_integrator == nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMBoundaryIntegratedBC creates the expected mfem::BoundaryIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMBoundaryIntegratedBC)
{
  // Build required BC inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericFunctorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"3.0"};
  _mfem_problem->addFunctorMaterial("MFEMGenericFunctorMaterial", "material1", coef_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMBoundaryIntegratedBC & integrated_bc =
      addObject<MFEMBoundaryIntegratedBC>("MFEMBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMBoundaryIntegratedBC returns an integrator of the expected type
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

TEST_F(MFEMIntegratedBCTest, MFEMVectorBoundaryIntegratedConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
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

TEST_F(MFEMIntegratedBCTest, MFEMVectorBoundaryIntegratedBC)
{
  // Build required BC inputs
  InputParameters func_params = _factory.getValidParams("ParsedVectorFunction");
  func_params.set<std::string>("expression_x") = "x + y";
  func_params.set<std::string>("expression_y") = "x + y + 1";
  func_params.set<std::string>("expression_z") = "x + y + 2";
  _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func1";
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

#endif
