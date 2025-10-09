#ifdef MOOSE_MFEM_ENABLED

#include "libmesh/ignore_warnings.h"
// #include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMObjectUnitTest.h"
#include "MFEMComplexScalarDirichletBC.h"
#include "MFEMComplexVectorNormalDirichletBC.h"
#include "MFEMComplexVectorTangentialDirichletBC.h"

class MFEMComplexEssentialBCTest : public MFEMObjectUnitTest
{
public:
  mfem::ParFiniteElementSpace _scalar_fes, _vector_h1_fes, _vector_hcurl_fes, _vector_hdiv_fes;
  mfem::ParComplexGridFunction _scalar_gridfunc, _vector_h1_gridfunc, _vector_hcurl_gridfunc,
      _vector_hdiv_gridfunc;
  mfem::ConstantCoefficient _scalar_zero;
  mfem::VectorConstantCoefficient _vector_zero;

  MFEMComplexEssentialBCTest()
    : MFEMObjectUnitTest("MooseUnitApp"),
      _scalar_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), new mfem::H1_FECollection(1, 3)),
      _vector_h1_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(),
                     new mfem::H1_FECollection(1, 3, mfem::BasisType::GaussLobatto),
                     3),
      _vector_hcurl_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), new mfem::ND_FECollection(2, 3)),
      _vector_hdiv_fes(_mfem_mesh_ptr->getMFEMParMeshPtr().get(), new mfem::RT_FECollection(2, 3)),
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
    _scalar_gridfunc.ProjectCoefficient(_scalar_zero, _scalar_zero);
    _vector_h1_gridfunc.ProjectCoefficient(_vector_zero, _vector_zero);
    _vector_hcurl_gridfunc.ProjectCoefficient(_vector_zero, _vector_zero);
    _vector_hdiv_gridfunc.ProjectCoefficient(_vector_zero, _vector_zero);
    // Register a dummy (Par)GridFunction for the variable the BCs apply to
    auto pcgf = std::make_shared<mfem::ParComplexGridFunction>(&_scalar_fes);
    _mfem_problem->getProblemData().cpx_gridfunctions.Register("test_cpx_variable_name", pcgf);
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

///**
// * Test MFEMScalarDirichletBC can be constructed from a constant and applied successfully
// */
TEST_F(MFEMComplexEssentialBCTest, MFEMComplexScalarDirichletConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMComplexScalarDirichletBC");
  bc_params.set<VariableName>("variable") = "test_cpx_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("coefficient_real") = "1.";
  bc_params.set<MFEMScalarCoefficientName>("coefficient_imag") = "1.";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMComplexScalarDirichletBC>("MFEMComplexScalarDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_cpx_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_cpx_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_scalar_gridfunc);

  // Check the correct boundary values have been applied
  mfem::GridFunctionCoefficient scalar_variable_real(&_scalar_gridfunc.real());
  mfem::GridFunctionCoefficient scalar_variable_imag(&_scalar_gridfunc.imag());
  check_boundary(
      1,
      _scalar_fes,
      [&scalar_variable_real](mfem::ElementTransformation * transform,
                              const mfem::IntegrationPoint & point)
      { return scalar_variable_real.Eval(*transform, point) - 1.; },
      1e-8);

  check_boundary(
      1,
      _scalar_fes,
      [&scalar_variable_imag](mfem::ElementTransformation * transform,
                              const mfem::IntegrationPoint & point)
      { return scalar_variable_imag.Eval(*transform, point) - 1.; },
      1e-8);
}

///**
// * Test MFEMScalarDirichletBC can be constructed and applied successfully
// */
TEST_F(MFEMComplexEssentialBCTest, MFEMComplexScalarDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMComplexScalarDirichletBC");
  bc_params.set<VariableName>("variable") = "test_cpx_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("coefficient_real") = "func1";
  bc_params.set<MFEMScalarCoefficientName>("coefficient_imag") = "func1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc =
      addObject<MFEMComplexScalarDirichletBC>("MFEMComplexScalarDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_cpx_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_cpx_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_scalar_gridfunc);

  // Check the correct boundary values have been applied
  mfem::GridFunctionCoefficient scalar_variable_real(&_scalar_gridfunc.real());
  mfem::GridFunctionCoefficient scalar_variable_imag(&_scalar_gridfunc.imag());
  mfem::Coefficient & expected(_mfem_problem->getCoefficients().getScalarCoefficient("func1"));
  check_boundary(
      1,
      _scalar_fes,
      [&scalar_variable_real, &expected](mfem::ElementTransformation * transform,
                                         const mfem::IntegrationPoint & point)
      { return scalar_variable_real.Eval(*transform, point) - expected.Eval(*transform, point); },
      1e-8);
  check_boundary(
      1,
      _scalar_fes,
      [&scalar_variable_imag, &expected](mfem::ElementTransformation * transform,
                                         const mfem::IntegrationPoint & point)
      { return scalar_variable_imag.Eval(*transform, point) - expected.Eval(*transform, point); },
      1e-8);
}

///**
// * Test MFEMVectorNormalDirichletBC can be constructed from a constant and applied
// * successfully
// */
TEST_F(MFEMComplexEssentialBCTest, MFEMComplexVectorNormalDirichletConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMComplexVectorNormalDirichletBC");
  bc_params.set<VariableName>("variable") = "test_cpx_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_real") = "1. 2. 3.";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_imag") = "1. 2. 3.";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMComplexVectorNormalDirichletBC>(
      "MFEMComplexVectorNormalDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_cpx_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_cpx_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hdiv_gridfunc);

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable_real(&_vector_hdiv_gridfunc.real());
  mfem::VectorGridFunctionCoefficient variable_imag(&_vector_hdiv_gridfunc.imag());
  mfem::Vector assigned_val({1., 2., 3.});
  check_boundary(
      1,
      _vector_hdiv_fes,
      [this, &variable_real, &assigned_val](mfem::ElementTransformation * transform,
                                            const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform);
        variable_real.Eval(actual, *transform, point);
        expected = assigned_val;
        actual -= expected;
        // (actual - expected) should be perpendicular to the normal and have a dot product of 0.
        return normal[0] * actual[0] + normal[1] * actual[1] + normal[2] * actual[2];
      },
      1e-8);
  check_boundary(
      1,
      _vector_hdiv_fes,
      [this, &variable_imag, &assigned_val](mfem::ElementTransformation * transform,
                                            const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform);
        variable_imag.Eval(actual, *transform, point);
        expected = assigned_val;
        actual -= expected;
        // (actual - expected) should be perpendicular to the normal and have a dot product of 0.
        return normal[0] * actual[0] + normal[1] * actual[1] + normal[2] * actual[2];
      },
      1e-8);
}

///**
// * Test MFEMVectorNormalDirichletBC can be constructed and applied successfully
// */
TEST_F(MFEMComplexEssentialBCTest, MFEMComplexVectorNormalDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMComplexVectorNormalDirichletBC");
  bc_params.set<VariableName>("variable") = "test_cpx_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_real") = "func2";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_imag") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMComplexVectorNormalDirichletBC>(
      "MFEMComplexVectorNormalDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_cpx_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_cpx_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hdiv_gridfunc);

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable_real(&_vector_hdiv_gridfunc.real());
  mfem::VectorGridFunctionCoefficient variable_imag(&_vector_hdiv_gridfunc.imag());
  mfem::VectorCoefficient & function(
      _mfem_problem->getCoefficients().getVectorCoefficient("func2"));
  check_boundary(
      1,
      _vector_hdiv_fes,
      [this, &variable_real, &function](mfem::ElementTransformation * transform,
                                        const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform);
        variable_real.Eval(actual, *transform, point);
        function.Eval(expected, *transform, point);
        actual -= expected;
        // (actual - expected) should be perpendicular to the normal and have a dot product of 0.
        return normal[0] * actual[0] + normal[1] * actual[1] + normal[2] * actual[2];
      },
      1e-8);
  check_boundary(
      1,
      _vector_hdiv_fes,
      [this, &variable_imag, &function](mfem::ElementTransformation * transform,
                                        const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform);
        variable_imag.Eval(actual, *transform, point);
        function.Eval(expected, *transform, point);
        actual -= expected;
        // (actual - expected) should be perpendicular to the normal and have a dot product of 0.
        return normal[0] * actual[0] + normal[1] * actual[1] + normal[2] * actual[2];
      },
      1e-8);
}

///**
// * Test MFEMVectorTangentialDirichletBC can be constructed from a constant and applied
// * successfully
// */
TEST_F(MFEMComplexEssentialBCTest, MFEMComplexVectorTangentialDirichletConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMComplexVectorTangentialDirichletBC");
  bc_params.set<VariableName>("variable") = "test_cpx_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_real") = "1. 2. 3.";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_imag") = "1. 2. 3.";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMComplexVectorTangentialDirichletBC>(
      "MFEMComplexVectorTangentialDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_cpx_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_cpx_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hcurl_gridfunc);
  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable_real(&_vector_hcurl_gridfunc.real());
  mfem::VectorGridFunctionCoefficient variable_imag(&_vector_hcurl_gridfunc.imag());
  mfem::Vector expected({1., 2., 3.});
  check_boundary(
      1,
      _vector_hcurl_fes,
      [this, &variable_real, &expected](mfem::ElementTransformation * transform,
                                        const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), normal = calc_normal(transform), cross_prod(3);
        variable_real.Eval(actual, *transform, point);
        actual -= expected;
        // (actual - expected) should be parallel to the normal and have a cross product of 0.
        cross_prod = normal[1] * actual[2] - normal[2] * actual[1];
        cross_prod = normal[2] * actual[0] - normal[0] * actual[2];
        cross_prod = normal[0] * actual[1] - normal[1] * actual[0];
        return cross_prod.Norml2();
      },
      1e-8);
  check_boundary(
      1,
      _vector_hcurl_fes,
      [this, &variable_imag, &expected](mfem::ElementTransformation * transform,
                                        const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), normal = calc_normal(transform), cross_prod(3);
        variable_imag.Eval(actual, *transform, point);
        actual -= expected;
        // (actual - expected) should be parallel to the normal and have a cross product of 0.
        cross_prod = normal[1] * actual[2] - normal[2] * actual[1];
        cross_prod = normal[2] * actual[0] - normal[0] * actual[2];
        cross_prod = normal[0] * actual[1] - normal[1] * actual[0];
        return cross_prod.Norml2();
      },
      1e-8);
}

///**
// * Test MFEMVectorTangentialDirichletBC can be constructed and applied successfully
// */
TEST_F(MFEMComplexEssentialBCTest, MFEMComplexVectorTangentialDirichletBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMComplexVectorTangentialDirichletBC");
  bc_params.set<VariableName>("variable") = "test_cpx_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_imag") = "func2";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient_real") = "func2";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  auto & essential_bc = addObject<MFEMComplexVectorTangentialDirichletBC>(
      "MFEMComplexVectorTangentialDirichletBC", "bc1", bc_params);

  EXPECT_EQ(essential_bc.getTrialVariableName(), "test_cpx_variable_name");
  EXPECT_EQ(essential_bc.getTestVariableName(), "test_cpx_variable_name");

  // Test applying the BC
  essential_bc.ApplyBC(_vector_hcurl_gridfunc);

  // Check the correct boundary values have been applied
  mfem::VectorGridFunctionCoefficient variable_real(&_vector_hcurl_gridfunc.real());
  mfem::VectorGridFunctionCoefficient variable_imag(&_vector_hcurl_gridfunc.imag());
  mfem::VectorCoefficient & function(
      _mfem_problem->getCoefficients().getVectorCoefficient("func2"));
  check_boundary(
      1,
      _vector_hcurl_fes,
      [this, &variable_real, &function](mfem::ElementTransformation * transform,
                                        const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform), cross_prod(3);
        variable_real.Eval(actual, *transform, point);
        function.Eval(expected, *transform, point);
        actual -= expected;
        // (actual - expected) should be parallel to the normal and have a cross product of 0.
        cross_prod = normal[1] * actual[2] - normal[2] * actual[1];
        cross_prod = normal[2] * actual[0] - normal[0] * actual[2];
        cross_prod = normal[0] * actual[1] - normal[1] * actual[0];
        return cross_prod.Norml2();
      },
      1e-8);
  check_boundary(
      1,
      _vector_hcurl_fes,
      [this, &variable_imag, &function](mfem::ElementTransformation * transform,
                                        const mfem::IntegrationPoint & point)
      {
        mfem::Vector actual(3), expected(3), normal = calc_normal(transform), cross_prod(3);
        variable_imag.Eval(actual, *transform, point);
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
