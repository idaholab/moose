//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMSumAux.h"
#include "MFEMComplexSumAux.h"
#include "MFEMComplexGradAux.h"
#include "MFEMComplexCurlAux.h"
#include "MFEMComplexDivAux.h"
#include "MFEMComplexDotProductAux.h"
#include "MFEMComplexCrossProductAux.h"
#include "MFEMComplexScalarProjectionAux.h"
#include "MFEMComplexVectorProjectionAux.h"

class MFEMAuxKernelTest : public MFEMObjectUnitTest
{
public:
  MFEMAuxKernelTest() : MFEMObjectUnitTest("MooseUnitApp") {}
};

double slope_real(const mfem::Vector &x)
{
  return x(0);
}

double slope_imag(const mfem::Vector &x)
{
  return x(1);
}

void vec_real(const mfem::Vector &x, mfem::Vector &v)
{
  for (int i=0; i<3; ++i)
    v(i) = i*x(i);
}

void vec_imag(const mfem::Vector &x, mfem::Vector &v)
{
  for (int i=0; i<3; ++i)
    v(i) = (i+1)*x(i);
}

/**
 * Test MFEMSumAux sums input GridFunctions successfully.
 */
TEST_F(MFEMAuxKernelTest, MFEMSumAux)
{
  // Register dummy (Par)GridFunctions to test MFEMSumAux against
  auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
  mfem::common::H1_ParFESpace fe(pm, 1);
  mfem::common::H1_ParFESpace different_fe(pm, 2);

  auto pgf_1 = std::make_shared<mfem::ParGridFunction>(&fe);
  auto pgf_2 = std::make_shared<mfem::ParGridFunction>(&fe);
  auto pgf_3 = std::make_shared<mfem::ParGridFunction>(&fe);
  auto pgf_out = std::make_shared<mfem::ParGridFunction>(&fe);
  auto pgf_ho = std::make_shared<mfem::ParGridFunction>(&different_fe);

  _mfem_problem->getProblemData().gridfunctions.Register("source_variable_1", pgf_1);
  _mfem_problem->getProblemData().gridfunctions.Register("source_variable_2", pgf_2);
  _mfem_problem->getProblemData().gridfunctions.Register("source_variable_3", pgf_3);
  _mfem_problem->getProblemData().gridfunctions.Register("summed_variable", pgf_out);
  _mfem_problem->getProblemData().gridfunctions.Register("source_ho_variable", pgf_ho);

  // Initialise variables that are to be summed over
  mfem::ConstantCoefficient initial_condition_coef(2.0);
  pgf_1->ProjectCoefficient(initial_condition_coef);
  pgf_2->ProjectCoefficient(initial_condition_coef);
  pgf_3->ProjectCoefficient(initial_condition_coef);
  pgf_ho->ProjectCoefficient(initial_condition_coef);

  {
    // Construct auxkernel
    InputParameters auxkernel_params = _factory.getValidParams("MFEMSumAux");
    auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
    auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
        "source_variable_1", "source_variable_2", "source_variable_3"};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors") = {1.0, 2.0, 5.0};

    MFEMSumAux & auxkernel = addObject<MFEMSumAux>("MFEMSumAux", "auxkernel1", auxkernel_params);
    auxkernel.execute();

    // Check the value of the output gridfunction is the scaled sum of the components
    ASSERT_EQ(pgf_out->GetData()[5], 16.0);
  }
  {
    // Check for failure if a variable to be summed has a different FESpace from the parent
    InputParameters auxkernel_params = _factory.getValidParams("MFEMSumAux");
    auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
    auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
        "source_variable_1", "source_ho_variable", "source_variable_3"};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors") = {1.0, 2.0, 5.0};
    EXPECT_THROW(addObject<MFEMSumAux>("MFEMSumAux", "failed_auxkernel", auxkernel_params),
                 std::runtime_error);
  }
  {
    // Check for failure if an inconsistent number of scale factors are provided
    InputParameters auxkernel_params = _factory.getValidParams("MFEMSumAux");
    auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
    auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
        "source_variable_1", "source_variable_2", "source_variable_3"};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors") = {1.0, 2.0};
    EXPECT_THROW(addObject<MFEMSumAux>("MFEMSumAux", "failed_scaled_auxkernel", auxkernel_params),
                 std::runtime_error);
  }
}


/**
 * Test MFEMComplexSumAux sums input ComplexGridFunctions successfully.
 */
TEST_F(MFEMAuxKernelTest, MFEMComplexSumAux)
{
  // Register dummy (Par)GridFunctions to test MFEMComplexSumAux against
  auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
  mfem::common::H1_ParFESpace fe(pm, 1);
  mfem::common::H1_ParFESpace different_fe(pm, 2);

  auto pgf_1 = std::make_shared<mfem::ParComplexGridFunction>(&fe);
  auto pgf_2 = std::make_shared<mfem::ParComplexGridFunction>(&fe);
  auto pgf_3 = std::make_shared<mfem::ParComplexGridFunction>(&fe);
  auto pgf_out = std::make_shared<mfem::ParComplexGridFunction>(&fe);
  auto pgf_ho = std::make_shared<mfem::ParComplexGridFunction>(&different_fe);

  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable_1", pgf_1);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable_2", pgf_2);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable_3", pgf_3);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("summed_variable", pgf_out);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_ho_variable", pgf_ho);

  // Initialise variables that are to be summed over
  mfem::ConstantCoefficient initial_condition_coef_real(2.0);
  mfem::ConstantCoefficient initial_condition_coef_imag(3.0);
  pgf_1->ProjectCoefficient(initial_condition_coef_real, initial_condition_coef_imag);
  pgf_2->ProjectCoefficient(initial_condition_coef_real, initial_condition_coef_imag);
  pgf_3->ProjectCoefficient(initial_condition_coef_real, initial_condition_coef_imag);
  pgf_ho->ProjectCoefficient(initial_condition_coef_real, initial_condition_coef_imag);

  {
    // Construct auxkernel
    InputParameters auxkernel_params = _factory.getValidParams("MFEMComplexSumAux");
    auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
    auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
        "source_variable_1", "source_variable_2", "source_variable_3"};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors_real") = {1.0, 2.0, 5.0};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors_imag") = {0.0, 2.0, -1.0};

    MFEMComplexSumAux & auxkernel = addObject<MFEMComplexSumAux>("MFEMComplexSumAux", "auxkernel1", auxkernel_params);
    auxkernel.execute();

    // Check the value of the output gridfunction is the scaled sum of the components
    ASSERT_EQ(pgf_out->real().GetData()[5], 13.0);
    ASSERT_EQ(pgf_out->imag().GetData()[5], 26.0);
  }
  {
    // Check for failure if a variable to be summed has a different FESpace from the parent
    InputParameters auxkernel_params = _factory.getValidParams("MFEMComplexSumAux");
    auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
    auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
        "source_variable_1", "source_ho_variable", "source_variable_3"};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors_real") = {1.0, 2.0, 5.0};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors_imag") = {0.0, 2.0, -1.0};
    EXPECT_THROW(addObject<MFEMComplexSumAux>("MFEMComplexSumAux", "failed_auxkernel", auxkernel_params),
                 std::runtime_error);
  }
  {
    // Check for failure if an inconsistent number of scale factors are provided
    InputParameters auxkernel_params = _factory.getValidParams("MFEMComplexSumAux");
    auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
    auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
        "source_variable_1", "source_variable_2", "source_variable_3"};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors_real") = {1.0, 2.0};
    auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors_imag") = {0.0, 2.0};
    EXPECT_THROW(addObject<MFEMComplexSumAux>("MFEMComplexSumAux", "failed_scaled_auxkernel", auxkernel_params),
                 std::runtime_error);
  }
}


/**
 * Test the differential operator auxkernels work on ComplexGridFunctions.
 */
TEST_F(MFEMAuxKernelTest, MFEMComplexGradCurlDivAux)
{
  // Register dummy (Par)GridFunctions to test MFEMComplexGradAux against
  auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
  mfem::common::H1_ParFESpace fe_h1(pm, 2);
  mfem::common::ND_ParFESpace fe_hcurl(pm, 2, 3, mfem::Ordering::byVDIM);
  mfem::common::RT_ParFESpace fe_hdiv(pm, 2, 3, mfem::Ordering::byVDIM);
  mfem::common::L2_ParFESpace fe_l2(pm, 2, 3);

  auto pgf_in = std::make_shared<mfem::ParComplexGridFunction>(&fe_h1);
  auto pgf_in_vec = std::make_shared<mfem::ParComplexGridFunction>(&fe_hdiv);

  auto pgf_grad = std::make_shared<mfem::ParComplexGridFunction>(&fe_hcurl);
  auto pgf_curl = std::make_shared<mfem::ParComplexGridFunction>(&fe_hdiv);
  auto pgf_div = std::make_shared<mfem::ParComplexGridFunction>(&fe_l2);

  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable", pgf_in);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable_vec", pgf_in_vec);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_grad", pgf_grad);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_curl", pgf_curl);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_div", pgf_div);

  // Initialise scalar variable to take the grad of
  mfem::FunctionCoefficient coef_real(slope_real);
  mfem::FunctionCoefficient coef_imag(slope_imag);
  pgf_in->ProjectCoefficient(coef_real, coef_imag);

  // Initialise a vector variable to take the div of
  mfem::VectorFunctionCoefficient vec_coef_real(3, vec_real);
  mfem::VectorFunctionCoefficient vec_coef_imag(3, vec_imag);
  pgf_in_vec->ProjectCoefficient(vec_coef_real, vec_coef_imag);

  {
    // Construct grad auxkernel
    InputParameters auxkernel_grad_params = _factory.getValidParams("MFEMComplexGradAux");
    auxkernel_grad_params.set<AuxVariableName>("variable") = "output_variable_grad";
    auxkernel_grad_params.set<VariableName>("source") = "source_variable";
    auxkernel_grad_params.set<mfem::real_t>("scale_factor_real") = 1.0;
    auxkernel_grad_params.set<mfem::real_t>("scale_factor_imag") = 1.0;

    MFEMComplexGradAux & auxkernel_grad = addObject<MFEMComplexGradAux>("MFEMComplexGradAux", "auxkernel1", auxkernel_grad_params);
    auxkernel_grad.execute();

    // Check we get the right grad
    ASSERT_LE(abs(pgf_grad->real().GetData()[0] - 0.0), 1e-8);
    ASSERT_LE(abs(pgf_grad->real().GetData()[1] - 0.0), 1e-8);
    ASSERT_LE(abs(pgf_grad->real().GetData()[2] + 1.0), 1e-8);
    ASSERT_LE(abs(pgf_grad->imag().GetData()[0] - 2.0), 1e-8);
    ASSERT_LE(abs(pgf_grad->imag().GetData()[1] - 2.0), 1e-8);
    ASSERT_LE(abs(pgf_grad->imag().GetData()[2] - 1.0), 1e-8);

    // Construct curl auxkernel
    InputParameters auxkernel_curl_params = _factory.getValidParams("MFEMComplexCurlAux");
    auxkernel_curl_params.set<AuxVariableName>("variable") = "output_variable_curl";
    auxkernel_curl_params.set<VariableName>("source") = "output_variable_grad";

    MFEMComplexCurlAux & auxkernel_curl = addObject<MFEMComplexCurlAux>("MFEMComplexCurlAux", "auxkernel2", auxkernel_curl_params);
    auxkernel_curl.execute();

    // Check we get the right curl (should be zero since curl of a grad = 0)
    ASSERT_LE(abs(pgf_curl->real().GetData()[0]), 1e-8);
    ASSERT_LE(abs(pgf_curl->real().GetData()[1]), 1e-8);
    ASSERT_LE(abs(pgf_curl->real().GetData()[2]), 1e-8);
    ASSERT_LE(abs(pgf_curl->imag().GetData()[0]), 1e-8);
    ASSERT_LE(abs(pgf_curl->imag().GetData()[1]), 1e-8);
    ASSERT_LE(abs(pgf_curl->imag().GetData()[2]), 1e-8);

    // Construct div auxkernel
    InputParameters auxkernel_div_params = _factory.getValidParams("MFEMComplexDivAux");
    auxkernel_div_params.set<AuxVariableName>("variable") = "output_variable_div";
    auxkernel_div_params.set<VariableName>("source") = "source_variable_vec";

    MFEMComplexDivAux & auxkernel_div = addObject<MFEMComplexDivAux>("MFEMComplexDivAux", "auxkernel3", auxkernel_div_params);
    auxkernel_div.execute();

    // Check the value of the output gridfunction is the correct div
    ASSERT_LE(abs(pgf_div->real().GetData()[5] - 3.0), 1e-8);
    ASSERT_LE(abs(pgf_div->imag().GetData()[5] - 6.0), 1e-8);
  }
}

/**
 * Test the complex dot and cross product auxkernels work on ComplexGridFunctions.
 */
TEST_F(MFEMAuxKernelTest, MFEMComplexDotCrossAux)
{
  // Register dummy (Par)GridFunctions to test MFEMComplexGradAux against
  auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
  mfem::common::L2_ParFESpace fe_l2(pm, 2, 3);
  mfem::common::ND_ParFESpace fe_hcurl(pm, 2, 3, mfem::Ordering::byVDIM);
  mfem::common::L2_ParFESpace fe_l2_vec(pm, 2, 3, 3, mfem::Ordering::byVDIM);


  auto pgf_in_1 = std::make_shared<mfem::ParComplexGridFunction>(&fe_hcurl);
  auto pgf_in_2 = std::make_shared<mfem::ParComplexGridFunction>(&fe_hcurl);
  auto pgf_out_1 = std::make_shared<mfem::ParComplexGridFunction>(&fe_l2);
  auto pgf_out_2 = std::make_shared<mfem::ParComplexGridFunction>(&fe_l2_vec);

  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable_1", pgf_in_1);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("source_variable_2", pgf_in_2);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_inner", pgf_out_1);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_cross", pgf_out_2);

  // Initialise vector variables to take dot and cross products of
  mfem::Vector real_vec{0, 1, 2};
  mfem::Vector imag_vec{1, 2, 3};
  mfem::VectorConstantCoefficient vec_coef_real(real_vec);
  mfem::VectorConstantCoefficient vec_coef_imag(imag_vec);
  pgf_in_1->ProjectCoefficient(vec_coef_real, vec_coef_imag);
  pgf_in_2->ProjectCoefficient(vec_coef_real, vec_coef_imag);

  {
    // Construct inner product auxkernel
    InputParameters auxkernel_inner_params = _factory.getValidParams("MFEMComplexDotProductAux");
    auxkernel_inner_params.set<AuxVariableName>("variable") = "output_variable_inner";
    auxkernel_inner_params.set<VariableName>("first_source_vec") = "source_variable_1";
    auxkernel_inner_params.set<VariableName>("second_source_vec") = "source_variable_2";

    MFEMComplexDotProductAux & auxkernel_inner = addObject<MFEMComplexDotProductAux>("MFEMComplexDotProductAux", "auxkernel1", auxkernel_inner_params);
    auxkernel_inner.execute();

    // Check we get the right inner product
    ASSERT_LE(abs(pgf_out_1->real().GetData()[5] - 19.0), 1e-8);
    ASSERT_LE(abs(pgf_out_1->imag().GetData()[5] - 0.0), 1e-8);

    // Construct cross product auxkernel
    InputParameters auxkernel_cross_params = _factory.getValidParams("MFEMComplexCrossProductAux");
    auxkernel_cross_params.set<AuxVariableName>("variable") = "output_variable_cross";
    auxkernel_cross_params.set<VariableName>("first_source_vec") = "source_variable_1";
    auxkernel_cross_params.set<VariableName>("second_source_vec") = "source_variable_2";

    MFEMComplexCrossProductAux & auxkernel_cross = addObject<MFEMComplexCrossProductAux>("MFEMComplexCrossProductAux", "auxkernel2", auxkernel_cross_params);
    auxkernel_cross.execute();

    // Check we get the right cross product
    ASSERT_LE(abs(pgf_out_2->real().GetData()[0]), 1e-8);
    ASSERT_LE(abs(pgf_out_2->real().GetData()[1]), 1e-8);
    ASSERT_LE(abs(pgf_out_2->real().GetData()[2]), 1e-8);
    ASSERT_LE(abs(pgf_out_2->imag().GetData()[0]-2.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->imag().GetData()[1]+4.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->imag().GetData()[2]-2.0), 1e-8);

  }
}

/**
 * Test the complex scalar and vector projection auxkernels on ComplexGridFunctions.
 */
TEST_F(MFEMAuxKernelTest, MFEMComplexScalarVectorProjectionAux)
{
  // Register dummy (Par)GridFunctions to test MFEMComplexGradAux against
  auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
  mfem::common::H1_ParFESpace fe_h1(pm, 2, 3);
  mfem::common::L2_ParFESpace fe_l2_vec(pm, 2, 3, 3, mfem::Ordering::byVDIM);

  auto pgf_out_1 = std::make_shared<mfem::ParComplexGridFunction>(&fe_h1);
  auto pgf_out_2 = std::make_shared<mfem::ParComplexGridFunction>(&fe_l2_vec);

  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_scalar", pgf_out_1);
  _mfem_problem->getProblemData().cmplx_gridfunctions.Register("output_variable_vector", pgf_out_2);

  // Initialise coefficients to project
  mfem::ConstantCoefficient coef_real(2.0);
  mfem::ConstantCoefficient coef_imag(3.0);
  mfem::Vector real_vec{0, 1, 2};
  mfem::Vector imag_vec{1, 2, 3};
  mfem::VectorConstantCoefficient vec_coef_real(real_vec);
  mfem::VectorConstantCoefficient vec_coef_imag(imag_vec);

  _mfem_problem->getProblemData().coefficients.declareScalar<mfem::ConstantCoefficient>("coef_real", coef_real);
  _mfem_problem->getProblemData().coefficients.declareScalar<mfem::ConstantCoefficient>("coef_imag", coef_imag);
  _mfem_problem->getProblemData().coefficients.declareVector<mfem::VectorConstantCoefficient>("vec_coef_real", vec_coef_real);
  _mfem_problem->getProblemData().coefficients.declareVector<mfem::VectorConstantCoefficient>("vec_coef_imag", vec_coef_imag);

  {
    // Construct scalar projection auxkernel
    InputParameters auxkernel_scalar_params = _factory.getValidParams("MFEMComplexScalarProjectionAux");
    auxkernel_scalar_params.set<AuxVariableName>("variable") = "output_variable_scalar";
    auxkernel_scalar_params.set<MFEMScalarCoefficientName>("coefficient_real") = "coef_real";
    auxkernel_scalar_params.set<MFEMScalarCoefficientName>("coefficient_imag") = "coef_imag";

    MFEMComplexScalarProjectionAux & auxkernel_scalar = addObject<MFEMComplexScalarProjectionAux>("MFEMComplexScalarProjectionAux", "auxkernel1", auxkernel_scalar_params);
    auxkernel_scalar.execute();

    // Check we get the right projection
    ASSERT_LE(abs(pgf_out_1->real().GetData()[5] - 2.0), 1e-8);
    ASSERT_LE(abs(pgf_out_1->imag().GetData()[5] - 3.0), 1e-8);

    // Construct vector projection auxkernel
    InputParameters auxkernel_vector_params = _factory.getValidParams("MFEMComplexVectorProjectionAux");
    auxkernel_vector_params.set<AuxVariableName>("variable") = "output_variable_vector";
    auxkernel_vector_params.set<MFEMVectorCoefficientName>("vector_coefficient_real") = "vec_coef_real";
    auxkernel_vector_params.set<MFEMVectorCoefficientName>("vector_coefficient_imag") = "vec_coef_imag";

    MFEMComplexVectorProjectionAux & auxkernel_vector = addObject<MFEMComplexVectorProjectionAux>("MFEMComplexVectorProjectionAux", "auxkernel2", auxkernel_vector_params);
    auxkernel_vector.execute();

    // Check we get the right projection
    ASSERT_LE(abs(pgf_out_2->real().GetData()[0] - 0.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->real().GetData()[1] - 1.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->real().GetData()[2] - 2.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->imag().GetData()[0] - 1.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->imag().GetData()[1] - 2.0), 1e-8);
    ASSERT_LE(abs(pgf_out_2->imag().GetData()[2] - 3.0), 1e-8);

  }
}

#endif
