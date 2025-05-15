#ifdef MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEML2Error.h"
#include "MFEMVectorL2Error.h"

class MFEMPostprocessorTest : public MFEMObjectUnitTest
{
public:
  mfem::GridFunction *_scalar_var, *_vector_var;
  MFEMPostprocessorTest() : MFEMObjectUnitTest("MooseUnitApp")
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
    _scalar_var = _mfem_problem->getProblemData().gridfunctions.Get("scalar_var");
    InputParameters vector_params = _factory.getValidParams("MFEMVariable");
    vector_params.set<UserObjectName>("fespace") = "ND_vector";
    _mfem_problem->addVariable("MFEMVariable", "vector_var", vector_params);
    _vector_var = _mfem_problem->getProblemData().gridfunctions.Get("vector_var");
  }
};

/**
 * Test MFEML2Error can be constructed and returns the expected results when applied.
 */
TEST_F(MFEMPostprocessorTest, MFEML2Error)
{
  InputParameters pp_params = _factory.getValidParams("MFEML2Error");
  pp_params.set<MFEMScalarCoefficientName>("function") = "scalar_ones";
  pp_params.set<VariableName>("variable") = "scalar_var";
  auto & l2_pp = addObject<MFEML2Error>("MFEML2Error", "ppl2", pp_params);

  mfem::ConstantCoefficient twos(2.);
  _scalar_var->ProjectCoefficient(twos);
  EXPECT_GT(l2_pp.getValue(), 1.);

  _scalar_var->ProjectCoefficient(
      _mfem_problem->getCoefficients().getScalarCoefficient("scalar_ones"));
  EXPECT_LT(l2_pp.getValue(), 1e-12);
}

/**
 * Test a corresponding coefficient has been created for MFEML2Error.
 */
TEST_F(MFEMPostprocessorTest, MFEML2ErrorCoefficient)
{
  InputParameters pp_params = _factory.getValidParams("MFEML2Error");
  pp_params.set<MFEMScalarCoefficientName>("function") = "scalar_ones";
  pp_params.set<VariableName>("variable") = "scalar_var";
  _mfem_problem->addPostprocessor("MFEML2Error", "ppl2", pp_params);
  auto & l2_pp = _mfem_problem->getUserObject<MFEML2Error>("ppl2");
  auto & l2_coef = _mfem_problem->getCoefficients().getScalarCoefficient("ppl2");

  mfem::ConstantCoefficient twos(2.);
  _scalar_var->ProjectCoefficient(twos);
  l2_pp.getValue();

  mfem::IsoparametricTransformation fe_transform;
  mfem::IntegrationPoint point;
  point.Init(2);
  point.Set2(0., 0.);
  fe_transform.SetIdentityTransformation(mfem::Geometry::SQUARE);

  EXPECT_EQ(l2_coef.Eval(fe_transform, point), l2_pp.getCurrentValue());
}

/**
 * Test MFEMVectorL2Error can be constructed and returns the expected results when applied.
 */
TEST_F(MFEMPostprocessorTest, MFEMVectorL2Error)
{
  InputParameters pp_params = _factory.getValidParams("MFEMVectorL2Error");
  pp_params.set<MFEMVectorCoefficientName>("function") = "vector_ones";
  pp_params.set<VariableName>("variable") = "vector_var";
  auto & l2_pp = addObject<MFEMVectorL2Error>("MFEMVectorL2Error", "ppl2", pp_params);

  mfem::VectorConstantCoefficient twos(mfem::Vector({2., 2.}));
  _vector_var->ProjectCoefficient(twos);
  EXPECT_GT(l2_pp.getValue(), 1.);

  _vector_var->ProjectCoefficient(
      _mfem_problem->getCoefficients().getVectorCoefficient("vector_ones"));
  EXPECT_LT(l2_pp.getValue(), 1e-12);
}

#endif
