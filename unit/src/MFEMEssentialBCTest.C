#include "mfem-common.hpp"
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
  mfem::common::H1_FESpace _scalar_fes;
  //  mfem::common::ND_FESpace _vector_fes;
  mfem::common::H1_FESpace _vector_fes;
  mfem::GridFunction _scalar_gridfunc, _vector_gridfunc;

  MFEMEssentialBCTest()
    : MFEMObjectUnitTest("PlatypusApp"),
      _scalar_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), 1, 3),
      // _vector_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), 1, 3, 3),
      _vector_fes(
          _mfem_mesh_ptr->getMFEMParMeshPtr().get(), 1, 3, mfem::BasisType::GaussLobatto, 3),
      _scalar_gridfunc(&_scalar_fes),
      _vector_gridfunc(&_vector_fes)
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
  essential_bc.ApplyBC(_scalar_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::Array<int> ess_vdofs_list, ess_vdofs_marker;
  _scalar_fes.GetEssentialVDofs(essential_bc._bdr_markers, ess_vdofs_marker);
  _scalar_fes.MarkerToList(ess_vdofs_marker, ess_vdofs_list);
  const Real expected = bc_params.get<Real>("value");
  for (auto ess_dof : ess_vdofs_list)
  {
    EXPECT_EQ(_scalar_gridfunc[ess_dof], expected);
  }
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
  essential_bc.ApplyBC(_scalar_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
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
  essential_bc.ApplyBC(_vector_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // Check the correct boundary values have been applied
  mfem::Array<int> ess_vdofs_list, ess_vdofs_marker;
  const std::vector<Real> expected = bc_params.get<std::vector<Real>>("values");
  for (int i = 0; i < 3; i++)
  {
    _vector_fes.GetEssentialVDofs(essential_bc._bdr_markers, ess_vdofs_marker, i);
    _vector_fes.MarkerToList(ess_vdofs_marker, ess_vdofs_list);
    for (auto ess_dof : ess_vdofs_list)
    {
      EXPECT_EQ(_vector_gridfunc[ess_dof], expected[i]);
    }
  }
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
  essential_bc.ApplyBC(_vector_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
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
  essential_bc.ApplyBC(_vector_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // Check the correct boundary values have been applied
  mfem::Array<int> ess_vdofs_list, ess_vdofs_marker;
  const std::vector<Real> expected = bc_params.get<std::vector<Real>>("values");
  for (int i = 0; i < 3; i++)
  {
    _vector_fes.GetEssentialVDofs(essential_bc._bdr_markers, ess_vdofs_marker, i);
    _vector_fes.MarkerToList(ess_vdofs_marker, ess_vdofs_list);
    for (auto ess_dof : ess_vdofs_list)
    {
      std::cout << "Index: " << ess_dof << std::endl;
      EXPECT_EQ(_vector_gridfunc[ess_dof], expected[i]);
    }
  }
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
  essential_bc.ApplyBC(_vector_gridfunc, _mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // FIXME: We should actually check this applies the right boundary values...
}
