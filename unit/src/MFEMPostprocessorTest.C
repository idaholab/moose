#include "MFEMObjectUnitTest.h"
#include "MFEML2Error.h"
#include "MFEMVectorL2Error.h"

class MFEMPostprocessorTest : public MFEMObjectUnitTest
{
public:
  mfem::GridFunction *_scalar_var, *_vector_var;
  MFEMPostprocessorTest() : MFEMObjectUnitTest("PlatypusApp")
  {
    InputParameters func_params = _factory.getValidParams("ParsedFunction");
    func_params.set<std::string>("expression") = "1";
    _mfem_problem->addFunction("ParsedFunction", "scalar_ones", func_params);
    _mfem_problem->getFunction("scalar_ones").initialSetup();
    InputParameters vecfunc_params = _factory.getValidParams("ParsedVectorFunction");
    vecfunc_params.set<std::string>("expression_x") = "1";
    vecfunc_params.set<std::string>("expression_y") = "1";
    vecfunc_params.set<std::string>("expression_z") = "1";
    _mfem_problem->addFunction("ParsedVectorFunction", "vector_ones", vecfunc_params);
    _mfem_problem->getFunction("vector_ones").initialSetup();
    InputParameters H1FE_params = _factory.getValidParams("MFEMScalarFESpace");
    H1FE_params.set<MooseEnum>("fec_type") = "H1";
    _mfem_problem->addFESpace("MFEMScalarFESpace", "H1_scalar", H1FE_params);
    InputParameters NDFE_params = _factory.getValidParams("MFEMVectorFESpace");
    NDFE_params.set<MooseEnum>("fec_type") = "ND";
    _mfem_problem->addFESpace("MFEMVectorFESpace", "ND_vector", NDFE_params);
    InputParameters scalar_params = _factory.getValidParams("MFEMVariable");
    scalar_params.set<UserObjectName>("fespace") = "H1_scalar";
    _mfem_problem->addVariable("MFEMVariable", "scalar_var", scalar_params);
    _scalar_var = _mfem_problem->getProblemData()._gridfunctions.Get("scalar_var");
    InputParameters vector_params = _factory.getValidParams("MFEMVariable");
    vector_params.set<UserObjectName>("fespace") = "ND_vector";
    _mfem_problem->addVariable("MFEMVariable", "vector_var", vector_params);
    _vector_var = _mfem_problem->getProblemData()._gridfunctions.Get("vector_var");
  }
};

/**
 * Test MFEML2Error can be constructed and returns the expected results when applied.
 */
TEST_F(MFEMPostprocessorTest, MFEML2Error)
{
  InputParameters pp_params = _factory.getValidParams("MFEML2Error");
  pp_params.set<FunctionName>("function") = "scalar_ones";
  pp_params.set<VariableName>("variable") = "scalar_var";
  auto & l2_pp = addObject<MFEML2Error>("MFEML2Error", "ppl2", pp_params);

  mfem::ConstantCoefficient twos(2.);
  _scalar_var->ProjectCoefficient(twos);
  EXPECT_GT(l2_pp.getValue(), 1.);

  _scalar_var->ProjectCoefficient(*_mfem_problem->getScalarFunctionCoefficient("scalar_ones"));
  EXPECT_LT(l2_pp.getValue(), 1e-12);
}

/**
 * Test MFEMVectorL2Error can be constructed and returns the expected results when applied.
 */
TEST_F(MFEMPostprocessorTest, MFEMVectorL2Error)
{
  InputParameters pp_params = _factory.getValidParams("MFEMVectorL2Error");
  pp_params.set<FunctionName>("function") = "vector_ones";
  pp_params.set<VariableName>("variable") = "vector_var";
  auto & l2_pp = addObject<MFEMVectorL2Error>("MFEMVectorL2Error", "ppl2", pp_params);

  mfem::VectorConstantCoefficient twos(mfem::Vector({2., 2.}));
  _vector_var->ProjectCoefficient(twos);
  EXPECT_GT(l2_pp.getValue(), 1.);

  _vector_var->ProjectCoefficient(*_mfem_problem->getVectorFunctionCoefficient("vector_ones"));
  EXPECT_LT(l2_pp.getValue(), 1e-12);
}
