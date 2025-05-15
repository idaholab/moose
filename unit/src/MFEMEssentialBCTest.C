#ifdef MFEM_ENABLED

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMObjectUnitTest.h"
#include "MFEMScalarDirichletBC.h"
#include "MFEMScalarFunctorDirichletBC.h"
#include "MFEMVectorDirichletBC.h"
#include "MFEMVectorFunctorDirichletBC.h"
#include "MFEMVectorNormalDirichletBC.h"
#include "MFEMVectorFunctorNormalDirichletBC.h"
#include "MFEMVectorTangentialDirichletBC.h"
#include "MFEMVectorFunctorTangentialDirichletBC.h"

class MFEMEssentialBCTest : public MFEMObjectUnitTest
{
public:
  mfem::common::H1_FESpace _scalar_fes;
  mfem::common::H1_FESpace _vector_h1_fes;
  mfem::common::ND_FESpace _vector_hcurl_fes;
  mfem::common::RT_FESpace _vector_hdiv_fes;
  mfem::GridFunction _scalar_gridfunc, _vector_h1_gridfunc, _vector_hcurl_gridfunc,
      _vector_hdiv_gridfunc;
  mfem::ConstantCoefficient _scalar_zero;
  mfem::VectorConstantCoefficient _vector_zero;

  MFEMEssentialBCTest()
    : MFEMObjectUnitTest("MooseUnitApp"),
      _scalar_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), 1, 3),
      _vector_h1_fes(
          _mfem_mesh_ptr->getMFEMParMeshPtr().get(), 1, 3, mfem::BasisType::GaussLobatto, 3),
      _vector_hcurl_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), 2, 3),
      _vector_hdiv_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), 2, 3),
      _scalar_gridfunc(&_scalar_fes),
      _vector_h1_gridfunc(&_vector_h1_fes),
      _vector_hcurl_gridfunc(&_vector_hcurl_fes),
      _vector_hdiv_gridfunc(&_vector_hdiv_fes),
      _scalar_zero(0.),
      _vector_zero(mfem::Vector({0., 0., 0.}))
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
    _scalar_gridfunc.ProjectCoefficient(_scalar_zero);
    _vector_h1_gridfunc.ProjectCoefficient(_vector_zero);
    _vector_hcurl_gridfunc.ProjectCoefficient(_vector_zero);
    _vector_hdiv_gridfunc.ProjectCoefficient(_vector_zero);
  }

  void check_boundary(int /*bound*/,
                      mfem::FiniteElementSpace & fespace,
                      std::function<double(mfem::ElementTransformation *,
                                           const mfem::IntegrationPoint &)> error_func,
                      double tolerance)
  {
    for (int be = 0; be < _mfem_mesh_ptr->getMFEMParMeshPtr()->GetNBE(); be++)
    {
      mfem::Element * elem = _mfem_mesh_ptr->getMFEMParMeshPtr()->GetBdrElement(be);
      if (elem->GetAttribute() != 1)
        continue;
      mfem::ElementTransformation * transform =
          _mfem_mesh_ptr->getMFEMParMeshPtr()->GetBdrElementTransformation(be);
      const mfem::FiniteElement * fe = fespace.GetBE(be);
      const mfem::IntegrationRule & ir =
          mfem::IntRules.Get(fe->GetGeomType(), 2 * fe->GetOrder() + 2);
      double total_error = 0.0;
      for (int j = 0; j < ir.GetNPoints(); j++)
      {
        const mfem::IntegrationPoint point = ir.IntPoint(j);
        transform->SetIntPoint(&point);
        const double error = error_func(transform, point);
        total_error += error * error;
      }
      EXPECT_LT(total_error, tolerance);
    }
  }

  mfem::Vector calc_normal(mfem::ElementTransformation * transform) const
  {
    mfem::Vector normal(3);
    mfem::CalcOrtho(transform->Jacobian(), normal);
    return normal;
  }
};

/**
 * Test MFEMScalarDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMScalarDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMScalarDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<Real>("value") = 1.;
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMScalarDirichletBC>("MFEMScalarDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_scalar_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::GridFunctionCoefficient scalar_variable(&_scalar_gridfunc);
  check_boundary(
      1,
      _scalar_fes,
      [&scalar_variable](mfem::ElementTransformation * transform,
                         const mfem::IntegrationPoint & point)
      { return scalar_variable.Eval(*transform, point) - 1.; },
      1e-8);
}

/**
 * Test MFEMScalarFunctorDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMScalarFunctorDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMScalarFunctorDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("coefficient") = "func1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMScalarFunctorDirichletBC>("MFEMScalarFunctorDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_scalar_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::GridFunctionCoefficient scalar_variable(&_scalar_gridfunc);
  mfem::Coefficient & expected(_mfem_problem->getCoefficients().getScalarCoefficient("func1"));
  check_boundary(
      1,
      _scalar_fes,
      [&scalar_variable, &expected](mfem::ElementTransformation * transform,
                                    const mfem::IntegrationPoint & point)
      { return scalar_variable.Eval(*transform, point) - expected.Eval(*transform, point); },
      1e-8);
}

/**
 * Test MFEMVectorDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorDirichletBC>("MFEMVectorDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_h1_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable(&_vector_h1_gridfunc);
  mfem::Vector expected({1., 2., 3.});
  check_boundary(
      1,
      _vector_h1_fes,
      [&variable, &expected](mfem::ElementTransformation * transform,
                             const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3);
        variable.Eval(actual, *transform, point);
        actual -= expected;
        return actual.Norml2();
      },
      1e-8);
}

/**
 * Test MFEMVectorFunctorDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorFunctorDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctorDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMVectorFunctorDirichletBC>("MFEMVectorFunctorDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_h1_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable(&_vector_h1_gridfunc);
  mfem::VectorCoefficient & function(
      _mfem_problem->getCoefficients().getVectorCoefficient("func2"));
  check_boundary(
      1,
      _vector_h1_fes,
      [&variable, &function](mfem::ElementTransformation * transform,
                             const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3);
        variable.Eval(actual, *transform, point);
        function.Eval(expected, *transform, point);
        actual -= expected;
        return actual.Norml2();
      },
      1e-8);
}

/**
 * Test MFEMVectorNormalDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorNormalDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorNormalDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMVectorNormalDirichletBC>("MFEMVectorNormalDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hdiv_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable(&_vector_hdiv_gridfunc);
  mfem::Vector assigned_val({1., 2., 3.});
  check_boundary(
      1,
      _vector_hdiv_fes,
      [this, &variable, &assigned_val](mfem::ElementTransformation * transform,
                                       const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform);
        variable.Eval(actual, *transform, point);
        expected = assigned_val;
        actual -= expected;
        // (actual - expected) should be perpendicular to the normal and have a dot product of 0.
        return normal[0] * actual[0] + normal[1] * actual[1] + normal[2] * actual[2];
      },
      1e-8);
}

/**
 * Test MFEMVectorFunctorNormalDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorFunctorNormalDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctorNormalDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorFunctorNormalDirichletBC>(
      "MFEMVectorFunctorNormalDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hdiv_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable(&_vector_hdiv_gridfunc);
  mfem::VectorCoefficient & function(
      _mfem_problem->getCoefficients().getVectorCoefficient("func2"));
  check_boundary(
      1,
      _vector_hdiv_fes,
      [this, &variable, &function](mfem::ElementTransformation * transform,
                                   const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform);
        variable.Eval(actual, *transform, point);
        function.Eval(expected, *transform, point);
        actual -= expected;
        // (actual - expected) should be perpendicular to the normal and have a dot product of 0.
        return normal[0] * actual[0] + normal[1] * actual[1] + normal[2] * actual[2];
      },
      1e-8);
}

/**
 * Test MFEMVectorTangentialDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorTangentialDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorTangentialDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<Real>>("values") = {1., 2., 3.};
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorTangentialDirichletBC>(
      "MFEMVectorTangentialDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hcurl_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());
  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable(&_vector_hcurl_gridfunc);
  mfem::Vector expected({1., 2., 3.});
  check_boundary(
      1,
      _vector_hcurl_fes,
      [this, &variable, &expected](mfem::ElementTransformation * transform,
                                   const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), normal = calc_normal(transform), cross_prod(3);
        variable.Eval(actual, *transform, point);
        actual -= expected;
        // (actual - expected) should be parallel to the normal and have a cross product of 0.
        cross_prod = normal[1] * actual[2] - normal[2] * actual[1];
        cross_prod = normal[2] * actual[0] - normal[0] * actual[2];
        cross_prod = normal[0] * actual[1] - normal[1] * actual[0];
        return cross_prod.Norml2();
      },
      1e-8);
}

/**
 * Test MFEMVectorFunctorTangentialDirichletBC can be constructed and applied successfully
 */
TEST_F(MFEMEssentialBCTest, MFEMVectorFunctorTangentialDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorFunctorTangentialDirichletBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMVectorFunctorTangentialDirichletBC>(
      "MFEMVectorFunctorTangentialDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hcurl_gridfunc, *_mfem_mesh_ptr->getMFEMParMeshPtr().get());

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable(&_vector_hcurl_gridfunc);
  mfem::VectorCoefficient & function(
      _mfem_problem->getCoefficients().getVectorCoefficient("func2"));
  check_boundary(
      1,
      _vector_hcurl_fes,
      [this, &variable, &function](mfem::ElementTransformation * transform,
                                   const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform), cross_prod(3);
        variable.Eval(actual, *transform, point);
        function.Eval(expected, *transform, point);
        actual -= expected;
        // (actual - expected) should be parallel to the normal and have a cross product of 0.
        cross_prod = normal[1] * actual[2] - normal[2] * actual[1];
        cross_prod = normal[2] * actual[0] - normal[0] * actual[2];
        cross_prod = normal[0] * actual[1] - normal[1] * actual[0];
        return cross_prod.Norml2();
      },
      1e-8);
}

#endif
