#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMSumAux.h"

class MFEMAuxKernelTest : public MFEMObjectUnitTest
{
public:
  MFEMAuxKernelTest() : MFEMObjectUnitTest("MooseUnitApp") {}
};

/**
 * Test MFEMSumAux creates sums input GridFunctions successfully.
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

  // Check for failure if a variable to be summed has a different FESpace from the parent
  InputParameters auxkernel_params = _factory.getValidParams("MFEMSumAux");
  auxkernel_params.set<AuxVariableName>("variable") = "summed_variable";
  auxkernel_params.set<std::vector<VariableName>>("source_variables") = {
      "source_variable_1", "source_ho_variable", "source_variable_3"};
  auxkernel_params.set<std::vector<mfem::real_t>>("scale_factors") = {1.0, 2.0, 5.0};
  EXPECT_THROW(addObject<MFEMSumAux>("MFEMSumAux", "failed_auxkernel", auxkernel_params),
               std::runtime_error);
}
}

#endif
