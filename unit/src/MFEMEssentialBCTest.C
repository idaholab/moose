#include "MFEMObjectUnitTest.h"
#include "MFEMScalarDirichletBC.h"
#include "MFEMScalarFunctionDirichletBC.h"
#include "MFEMVectorDirichletBC.h"
#include "MFEMVectorFunctionDirichletBC.h"
#include "MFEMVectorTangentialDirichletBC.h"
#include "MFEMVectorFunctionTangentialDirichletBC.h"

class MFEMEssentialBCTest : public MFEMObjectUnitTest
{
public:
  mfem::LinearFECollection _fec;
  mfem::FiniteElementSpace _fes;
  mfem::GridFunction _gridfunc;

  MFEMEssentialBCTest()
    : MFEMObjectUnitTest("PlatypusApp"),
      _fec(),
      _fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), &_fec),
      _gridfunc(&_fes)
  {
    InputParameters func_params = _factory.getValidParams("ParsedFunction");
    func_params.set<std::string>("expression") = "x + y";
    _mfem_problem->addFunction("ParsedFunction", "func1", func_params);
    _mfem_problem->getFunction("func1").initialSetup();
    InputParameters vecfunc_params = _factory.getValidParams("ParsedVectorFunction");
    vecfunc_params.set<std::string>("expression_x") = "x + y";
    vecfunc_params.set<std::string>("expression_y") = "x + y + 1";
    vecfunc_params.set<std::string>("expression_z") = "x + y + 2";
    _mfem_problem->addFunction("ParsedVectorFunction", "func2", vecfunc_params);
    _mfem_problem->getFunction("func2").initialSetup();
  }
};

/**
 * Test MFEMScalarDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMScalarDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMScalarDirichletBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<Real>("value") = 1.;
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMScalarDirichletBC>("MFEMScalarDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}

/**
 * Test MFEMScalarFunctionDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMScalarFunctionDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMScalarFunctionDirichletBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<FunctionName>("function") = "func1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMScalarFunctionDirichletBC>("MFEMScalarFunctionDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}

/**
 * Test MFEMVectorDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorDirichletBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorDirichletBC>("MFEMVectorDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}

/**
 * Test MFEMVectorFunctionDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorFunctionDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctionDirichletBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<FunctionName>("function") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMVectorFunctionDirichletBC>("MFEMVectorFunctionDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}

/**
 * Test MFEMVectorTangentialDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorTangentialDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorTangentialDirichletBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorTangentialDirichletBC>(
      "MFEMVectorTangentialDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}

/**
 * Test MFEMVectorFunctionDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorFunctionTangentialDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctionTangentialDirichletBC");
  bc_params.set<std::string>("variable") = "test_variable_name";
  bc_params.set<FunctionName>("function") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorFunctionTangentialDirichletBC>(
      "MFEMVectorFunctionTangentialDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}
