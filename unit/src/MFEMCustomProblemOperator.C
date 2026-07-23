//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "libmesh/ignore_warnings.h"
#include "MFEMProblem.h"
#include "ProblemOperatorBuilderBase.h"
#include "ProblemOperator.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMObjectUnitTest.h"
#include "MooseUnitUtils.h"

#include <string>
#include <vector>

/**
 * Custom Dummy Operator with a basic solve
 * uses same problem as mfem ex0p
 */
class CustomDummyProblemOperator : public Moose::MFEM::ProblemOperator
{
private:
  int _dumm_var = 0;
  // The linear and bilinear forms
  mfem::ParBilinearForm * _a;
  mfem::ParLinearForm * _b;

  // The coefficient
  mfem::ConstantCoefficient _one;

  // The boundary conditions arrays
  mfem::Array<int> _boundary_dofs;

  // The operator and solution vectors (could
  // potentially use the ones in the base class)
  mfem::OperatorHandle _problem_operator; // The actual mfem problem operator
  mfem::Vector _B, _X;

public:
  // The constructor
  CustomDummyProblemOperator(MFEMProblem & problem);

  // Solve the equation
  virtual void Solve() override
  {
    // Test to check if correct solve is called
    _dumm_var = 10;

    // Set the operator and solve the equation
    _problem_data.jacobian_solver->SetOperator(*_problem_operator);
    _problem_data.jacobian_solver->GetSolver().Mult(_B, _X);

    // Set the data in the grid function
    const std::string _grid_function_name = "var0";
    auto _grid_function = _problem_data.gridfunctions.GetShared(_grid_function_name);
    _grid_function->SetFromTrueDofs(_X);
  };

  // Get the dummy Variable
  int & getDumVar() { return _dumm_var; };

  // Multiply by the operator
  void Mult(const mfem::Vector &, mfem::Vector &) const override {};
};

// The custom operator constructor
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0)
  : Moose::MFEM::ProblemOperator(prob0), _one(1.000)
{
  // Retrieve the FE-space and gridFunction
  const std::string _fe_space_name = "h1";
  const std::string _grid_function_name = "var0";
  auto _fes = prob0.getProblemData().fespaces.GetShared(_fe_space_name);
  auto _grid_function = prob0.getProblemData().gridfunctions.GetShared(_grid_function_name);

  // Boundary conditions
  *_grid_function = 0.00;
  _fes->GetBoundaryTrueDofs(_boundary_dofs);

  // Build the linear form
  _b = new mfem::ParLinearForm(&(*_fes));
  _b->AddDomainIntegrator(new mfem::DomainLFIntegrator(_one));
  _b->Assemble();

  // Build the bilinear form
  _a = new mfem::ParBilinearForm(&(*_fes));
  _a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  _a->Assemble();

  // Form the linear system
  _a->FormLinearSystem(_boundary_dofs, *_grid_function, *_b, _problem_operator, _X, _B);
};

namespace Moose::MFEM
{
/**
 * Custom Dummy Operator builder required to build MFEM Problem Operators
 * used by the executioner
 */
class ProblemOperatorBuilderCustomDummy : public ProblemOperatorBuilderBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = ProblemOperatorBuilderBase::validParams();
    return params;
  };

  ProblemOperatorBuilderCustomDummy(const InputParameters & parameters)
    : ProblemOperatorBuilderBase(parameters) {};

  ~ProblemOperatorBuilderCustomDummy() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & _mfem_problem) override
  {
    return std::make_shared<CustomDummyProblemOperator>(_mfem_problem);
  };
};

registerMooseObject("MooseApp", ProblemOperatorBuilderCustomDummy);
};

/*****************************************************************
 * User does not have to specify the things beyond this point the
 * rest is handled using standard input files, this only occurs in
 * the Unit-test
 *****************************************************************/
// The unit test
// itself
class MFEMCustomProbOperatorTest : public MFEMObjectUnitTest
{
public:
  // The test data
  std::shared_ptr<Moose::MFEM::ProblemOperatorBuilderBase> _problem_operator_builder;
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> _problem_operator;

  // The test constructor
  MFEMCustomProbOperatorTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    // Add an FE-space
    InputParameters fe_params = _factory.getValidParams("MFEMScalarFESpace");
    fe_params.set<MooseEnum>("fec_type") = "H1";
    _mfem_problem->addFESpace("MFEMScalarFESpace", "h1", fe_params);

    // Add a variable
    InputParameters variable_params = _factory.getValidParams("MFEMVariable");
    variable_params.set<MFEMFESpaceName>("fespace") = "h1";
    _mfem_problem->addVariable("MFEMVariable", "var0", variable_params);

    // Add a solver
    InputParameters solver_params = _factory.getValidParams("MFEMMUMPS");
    solver_params.set<int>("print_level") = 0;
    _mfem_problem->addMFEMSolver("MFEMMUMPS", "linearSolver0", solver_params);
    _mfem_problem->resolveMFEMSolvers();

    // Add the custom problem operator builder
    // then get it and build the operator
    InputParameters _problem_operator_params =
        _factory.getValidParams("ProblemOperatorBuilderCustomDummy");
    _mfem_problem->addMFEMProblemOperator(
        "ProblemOperatorBuilderCustomDummy", "custom_problem_operator", _problem_operator_params);
    _problem_operator_builder = _mfem_problem->getProblemOperatorBuilder();
    _problem_operator = _problem_operator_builder->createProblemOperator(*_mfem_problem);
  };

protected:
  template <typename T>
  std::shared_ptr<T>
  addSharedObject(const std::string & type, const std::string & name, InputParameters & params)
  {
    auto objects = _mfem_problem->addObject<T>(type, name, params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    return objects[0];
  }
};

TEST_F(MFEMCustomProbOperatorTest, TestMFEMCustomOperators)
{
  // Solver problem using the MOOSE-MFEM route
  _problem_operator->Solve();

  /**
   * Solve the equation using
   * the standard MFEM route
   */
  // Set the Mesh
  mfem::ParMesh & pmesh = _mfem_mesh_ptr->getMFEMParMesh();

  // Set FE-spaces
  mfem::H1_FECollection fec0(1, pmesh.Dimension());
  mfem::ParFiniteElementSpace fespace0(&pmesh, &fec0);

  // Dirchelet boundary condition
  mfem::Array<int> boundary_dofs0;
  fespace0.GetBoundaryTrueDofs(boundary_dofs0);
  mfem::ParGridFunction x_func(&fespace0);
  x_func = 0.0;

  // Setup the linear forms
  mfem::ConstantCoefficient one(1.0);
  mfem::ParLinearForm b_form(&fespace0);
  b_form.AddDomainIntegrator(new mfem::DomainLFIntegrator(one));
  b_form.Assemble();

  // Setup the bilinear forms
  mfem::ParBilinearForm a_form(&fespace0);
  a_form.AddDomainIntegrator(new mfem::DiffusionIntegrator);
  a_form.Assemble();

  // Form the linear system
  mfem::HypreParMatrix a_mat;
  mfem::Vector B_vec, X_vec;
  a_form.FormLinearSystem(boundary_dofs0, x_func, b_form, a_mat, X_vec, B_vec);

  // Set the linear solver and solve
  mfem::MUMPSSolver SolverDir(MPI_COMM_WORLD);
  SolverDir.SetOperator(a_mat);
  SolverDir.SetPrintLevel(0);

  // Recover solutions
  a_form.RecoverFEMSolution(X_vec, b_form, x_func);

  /**
   * Examine test results
   */
  // Check whether the Solver ran with a dummy var
  auto _problem_operator1 = std::static_pointer_cast<CustomDummyProblemOperator>(_problem_operator);
  EXPECT_EQ(_problem_operator1->getDumVar(), 10);

  // Check number of Vertices in the mesh
  EXPECT_EQ(pmesh.GetNV(), _mfem_mesh_ptr->getMFEMParMesh().GetNV());

  // Check number of DOFs on the boundary
  EXPECT_EQ(pmesh.bdr_attributes.Size(), _mfem_mesh_ptr->getMFEMParMesh().bdr_attributes.Size());

  // Check the error norm of the solution
  const std::string grid_function_name = "var0";
  auto grid_function = _mfem_problem->getProblemData().gridfunctions.GetShared(grid_function_name);
  x_func -= *grid_function;
  EXPECT_NEAR(x_func.Norml2(), 0, 1.0e-7);
};
#endif
