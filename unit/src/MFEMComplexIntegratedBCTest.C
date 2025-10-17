#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMComplexIntegratedBC.h"
#include "MFEMBoundaryNormalIntegratedBC.h"
#include "MFEMConvectiveHeatFluxBC.h"

class MFEMComplexIntegratedBCTest : public MFEMObjectUnitTest
{
public:
  MFEMComplexIntegratedBCTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    // Register a dummy ParComplexGridFunction for the variable the BCs apply to
    auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
    auto pgf = std::make_shared<mfem::ParComplexGridFunction>(
        new mfem::ParFiniteElementSpace(pm, new mfem::H1_FECollection(1, pm->Dimension())));
    _mfem_problem->getProblemData().cmplx_gridfunctions.Register("test_cmplx_variable_name", pgf);
  }
};

/**
 * Test MFEMComplexIntegratedBC creates a real and imaginary mfem::BoundaryNormalLFIntegrator
 * successfully.
 */
TEST_F(MFEMComplexIntegratedBCTest, MFEMComplexIntegratedLinearFormBC)
{
  // Construct boundary condition
  InputParameters bc_normal_params = _factory.getValidParams("MFEMBoundaryNormalIntegratedBC");
  InputParameters bc_complex_params = _factory.getValidParams("MFEMComplexIntegratedBC");

  bc_complex_params.set<VariableName>("variable") = "test_cmplx_variable_name";
  bc_normal_params.set<VariableName>("variable") = "test_cmplx_variable_name";
  bc_normal_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  bc_normal_params.set<std::vector<BoundaryName>>("boundary") = {"1"};

  MFEMBoundaryNormalIntegratedBC & normal_integrated_bc = addObject<MFEMBoundaryNormalIntegratedBC>(
      "MFEMBoundaryNormalIntegratedBC", "bc1", bc_normal_params);
  MFEMComplexIntegratedBC & complex_integrated_bc = addObject<MFEMComplexIntegratedBC>(
      "MFEMComplexIntegratedBC", "bc_complex", bc_complex_params);

  complex_integrated_bc.setRealBC(
      std::dynamic_pointer_cast<MFEMIntegratedBC>(normal_integrated_bc.getSharedPtr()));
  complex_integrated_bc.setImagBC(
      std::dynamic_pointer_cast<MFEMIntegratedBC>(normal_integrated_bc.getSharedPtr()));

  // Test the complex integrated BC returns integrators of the expected type
  auto lf_real =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(complex_integrated_bc.getRealLFIntegrator());
  ASSERT_TRUE(lf_real != nullptr);
  auto lf_imag =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(complex_integrated_bc.getImagLFIntegrator());
  ASSERT_TRUE(lf_imag != nullptr);
  auto blf_real = complex_integrated_bc.getRealBFIntegrator();
  ASSERT_TRUE(blf_real == nullptr);
  auto blf_imag = complex_integrated_bc.getImagBFIntegrator();
  ASSERT_TRUE(blf_imag == nullptr);

  delete lf_real;
  delete lf_imag;
  delete blf_real;
  delete blf_imag;
}

/**
 * Test MFEMComplexIntegratedBC creates a real and imaginary mfem::MFEMConvectiveHeatFluxBC
 * and their associated LFs and BLFs successfully.
 */
TEST_F(MFEMComplexIntegratedBCTest, MFEMComplexIntegratedBilinearFormBC)
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
  InputParameters flux_bc_params = _factory.getValidParams("MFEMConvectiveHeatFluxBC");
  flux_bc_params.set<VariableName>("variable") = "test_cmplx_variable_name";
  flux_bc_params.set<MFEMScalarCoefficientName>("heat_transfer_coefficient") = "htc";
  flux_bc_params.set<MFEMScalarCoefficientName>("T_infinity") = "Tinf";
  flux_bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};

  InputParameters bc_complex_params = _factory.getValidParams("MFEMComplexIntegratedBC");
  bc_complex_params.set<VariableName>("variable") = "test_cmplx_variable_name";

  MFEMConvectiveHeatFluxBC & flux_bc =
      addObject<MFEMConvectiveHeatFluxBC>("MFEMConvectiveHeatFluxBC", "bc1", flux_bc_params);
  MFEMComplexIntegratedBC & complex_integrated_bc = addObject<MFEMComplexIntegratedBC>(
      "MFEMComplexIntegratedBC", "bc_complex", bc_complex_params);

  complex_integrated_bc.setRealBC(
      std::dynamic_pointer_cast<MFEMIntegratedBC>(flux_bc.getSharedPtr()));
  complex_integrated_bc.setImagBC(
      std::dynamic_pointer_cast<MFEMIntegratedBC>(flux_bc.getSharedPtr()));

  // Test the complex integrated BC returns integrators of the expected type
  auto lf_real =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(complex_integrated_bc.getRealLFIntegrator());
  ASSERT_TRUE(lf_real != nullptr);
  auto lf_imag =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(complex_integrated_bc.getImagLFIntegrator());
  ASSERT_TRUE(lf_imag != nullptr);
  auto blf_real =
      dynamic_cast<mfem::BoundaryMassIntegrator *>(complex_integrated_bc.getRealBFIntegrator());
  ASSERT_TRUE(blf_real != nullptr);
  auto blf_imag =
      dynamic_cast<mfem::BoundaryMassIntegrator *>(complex_integrated_bc.getImagBFIntegrator());
  ASSERT_TRUE(blf_imag != nullptr);

  delete lf_real;
  delete lf_imag;
  delete blf_real;
  delete blf_imag;
}

#endif
