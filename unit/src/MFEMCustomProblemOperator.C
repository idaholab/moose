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
class CustomDummyProblemOperator :  public Moose::MFEM::ProblemOperator
{
  private:
    int dummVar=0;
    // The linear and bilinear forms
    mfem::ParBilinearForm *a;
    mfem::ParLinearForm *b;

    // The coefficient
    mfem::ConstantCoefficient one;

    // The boundary conditions arrays
    mfem::Array<int> boundary_dofs;

    // The operator and solution vectors (could
    // potentially use the ones in the base class)
    mfem::OperatorHandle probOp; // The actual mfem problem operator
    mfem::Vector B, X;

  public:

    // The constructor
    CustomDummyProblemOperator(MFEMProblem & prob0);

    //Solve the equation
    virtual void Solve() override
    {
      //Test to check if correct solve is called
      dummVar = 10;

      // Set the operator and solve the equation
      _problem_data.jacobian_solver->SetOperator(probOp);
   //   _problem_data.jacobian_solver->Mult(B,X);

      // Set the data in the grid function
      const std::string gFuncName="var0";
      auto gfunc = _problem_data.gridfunctions.GetShared(gFuncName);
      *gfunc = 0.00;
    };

    //Get the dummy Variable
    int& GetDumVar(){return dummVar;};

    //Multiply by the operator
    void Mult(const mfem::Vector&, mfem::Vector&) const override {};
};

// The custom operator constructor
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0)
  : Moose::MFEM::ProblemOperator(prob0)
{
  // Retrieve the FE-space and gridFunction
  const std::string FEspaceName="h1";
  const std::string gFuncName="var0";
  auto fes   = prob0.getProblemData().fespaces.GetShared(FEspaceName);
  auto gfunc = prob0.getProblemData().gridfunctions.GetShared(gFuncName);

  // Boundary conditions
  *gfunc = 0.00;
  fes->GetBoundaryTrueDofs(boundary_dofs);

  // Build the linear form
  b = new mfem::ParLinearForm(&(*fes));
  b->AddDomainIntegrator(new mfem::DomainLFIntegrator(one));
  b->Assemble();

  // Build the bilinear form
  a = new mfem::ParBilinearForm(&(*fes));
  a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  a->Assemble();

  // Form the linear system
  a->FormLinearSystem(boundary_dofs, *gfunc, *b, probOp, X, B);
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
   : ProblemOperatorBuilderBase(parameters){};

  ~ProblemOperatorBuilderCustomDummy() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> createProblemOperator(MFEMProblem & mfemProb) override
  {
    return std::make_shared<CustomDummyProblemOperator>(mfemProb);
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
/*
class MFEMCustomProbOperatorTest : public MFEMObjectUnitTest
{
public:
  // The test data
  std::shared_ptr<Moose::MFEM::ProblemOperatorBuilderBase> probOpBuilder;
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> probOp;

  // The test constructor
  MFEMProbOperatorTest() : MFEMObjectUnitTest("MooseUnitApp")
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
    solver_params.set<mfem::real_t>("print_level") = 0;
    _mfem_problem->addMFEMSolver("MFEMMUMPS","linearSolver0",solver_params);

    // Add the custom problem operator builder
    // then get it and build the operator
    InputParameters prob_Op_params = _factory.getValidParams("ProblemOperatorBuilderCustomDummy");
    _mfem_problem->addMFEMProblemOperator("ProblemOperatorBuilderCustomDummy","cust_probOp", prob_Op_params);
    probOpBuilder = _mfem_problem->getProblemOperatorBuilder();
    probOp = probOpBuilder->createProblemOperator(*_mfem_problem);
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
  probOp->Solve();

  // Examine test results
  EXPECT_EQ( probOp->GetDumVar(), 10);
//  EXPECT_NEAR(u1.Norml2(), 0, 1e-5);
};
*/
#endif
