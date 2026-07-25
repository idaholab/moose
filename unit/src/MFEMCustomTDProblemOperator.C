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
#include "ProblemComposerBase.h"
#include "TimeDependentProblemOperator.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMObjectUnitTest.h"
#include "MooseUnitUtils.h"

#include <string>
#include <vector>

/**
 * Initial temperature distribution from
 * ex16p
 */
mfem::real_t InitialTemperature(const mfem::Vector &x)
{
   if (x.Norml2() < 0.5)
   {
      return 2.0;
   }
   else
   {
      return 1.0;
   }
}

/**
 * Custom Dummy Operator with a basic solve
 * uses same problem as mfem ex16p
 */
class CustomDummyTimeDependantProblemOperator : public Moose::MFEM::TimeDependentProblemOperator
{
private:
  int _dumm_var = 0;

  // The bilinear forms
  mfem::ParBilinearForm *_M=NULL, *_K=NULL;

  // The constants
  mfem::real_t _current_dt;
  mfem::real_t _alpha, _kappa;

  // GridFunctions and Coefficients for
  // non-linear-vars, BC's and IC's
  mfem::ParGridFunction *_u_alpha_gf;
  mfem::GridFunctionCoefficient *_u_coeff;
  mfem::FunctionCoefficient u_0;

  // The Dirch BC empty list for pure Neumann BC
  mfem::Array<int> _ess_tdof_list;

  // The operator and solution vectors (could
  // potentially use the ones in the base class)
  mfem::HypreParMatrix _M_mat, _K_mat;
  mfem::HypreParMatrix *_T; // T = M + dt K
  mutable mfem::Vector _z;

  // Solvers for inverting some operators
  mfem::CGSolver *M_solver;    // Krylov solver for inverting the mass matrix M
  mfem::HypreSmoother *M_prec; // Preconditioner for the mass matrix M

  // LSolver tolerance for Mass matrix inversion
  const mfem::real_t rel_tol = 1e-8;
public:
  // Get the dummy variable
  int& GetDumVar(){ return _dumm_var;};

  // The constructor
  CustomDummyTimeDependantProblemOperator(MFEMProblem & prob0);

  // Builder function cause ex16p is partially nonlinear
  void SetParameters(const mfem::Vector &u);

  // Initialise the problem
  virtual void Init(mfem::BlockVector &x) override;

  // Solves the backward-Euler equation k = f(u + dt*k, t)
  virtual void ImplicitSolve(const mfem::real_t dt, const mfem::Vector &u, mfem::Vector &k) override;

  // The function dudt = k = f(u)
  // in this case : k = - [M]^-1 [K(u)] u
  virtual void Mult(const mfem::Vector &x, mfem::Vector &y) const override;

  // Solve a single time-step u(x,t+dt) = G(u(x,t),...)
  virtual void Solve() override;
};

// The custom operator constructor
CustomDummyTimeDependantProblemOperator::CustomDummyTimeDependantProblemOperator(MFEMProblem & prob0)
  : Moose::MFEM::TimeDependentProblemOperator(prob0), u_0(InitialTemperature)
{
  // Retrieve the FE-space and gridFunction
  const std::string _fe_space_name = "h1";
  auto _fes = prob0.getProblemData().fespaces.GetShared(_fe_space_name);
  _u_alpha_gf = new mfem::ParGridFunction(&(*_fes));

  // Set the coefficients
  _u_coeff = new mfem::GridFunctionCoefficient(&(*_u_alpha_gf));

  // The diffusion term
  _K = new mfem::ParBilinearForm(&(*_fes));
  _K->AddDomainIntegrator(new mfem::DiffusionIntegrator(*_u_coeff));
  _K->Assemble(0); // keep sparsity pattern of M and K the same
  _K->FormSystemMatrix(_ess_tdof_list, _K_mat);

  // The mass term
  _M = new mfem::ParBilinearForm(&(*_fes));
  _M->AddDomainIntegrator(new mfem::MassIntegrator());
  _M->Assemble(0); // keep sparsity pattern of M and K the same
  _M->FormSystemMatrix(_ess_tdof_list, _M_mat);

  // As the Mass matrix is constant we need a inverse Mass
  // solver (this potentially could be done with the solver
  // system but this way is also possible)
  M_solver = new mfem::CGSolver(_fes->GetComm());
  M_prec = new mfem::HypreSmoother();
  M_prec->SetType(mfem::HypreSmoother::Jacobi);
  M_solver->iterative_mode = false;
  M_solver->SetRelTol(rel_tol);
  M_solver->SetAbsTol(0.0);
  M_solver->SetMaxIter(100);
  M_solver->SetPrintLevel(0);
  M_solver->SetPreconditioner(*M_prec);
  M_solver->SetOperator(_M_mat);
};

// Initialise the problem
void CustomDummyTimeDependantProblemOperator::Init(mfem::BlockVector &x)
{
  //Set the base class init
  TimeDependentProblemOperator::Init(x);

  // Set timestepper
  auto & ode_solver = _problem_data.ode_solver;
  ode_solver = std::make_unique<mfem::BackwardEulerSolver>();
  ode_solver->Init(*(this));
  SetTime(_problem.time());
  SetImplicitVariableType(STATE);

  // Set the initial conditions
  const std::string _grid_function_name = "var0";
  auto _grid_function = _problem_data.gridfunctions.GetShared(_grid_function_name);
  _grid_function->ProjectCoefficient(u_0);

  // Rebuild the operator with initial
  // conditions
  SetParameters(*_grid_function);
}


// Builder function cause ex16p is partially nonlinear
void CustomDummyTimeDependantProblemOperator::SetParameters(const mfem::Vector &u)
{
  // Update the internal grid function
  // used for the coefficient
  _u_alpha_gf->SetFromTrueDofs(u);
  for (int i = 0; i < _u_alpha_gf->Size(); i++)
  {
    (*_u_alpha_gf)(i) = _kappa + _alpha*(*_u_alpha_gf)(i);
  }

  // Reassemble the nonlinear-diffusion form
  _K->Update();
  _K->Assemble(0); // keep sparsity pattern of M and K the same
  _K->FormSystemMatrix(_ess_tdof_list, _K_mat);

  // Reset the T-Operator
  if(_T != NULL)
  {
    delete _T;
    _T = NULL;
  }
}

// Solves the backward-Euler equation k = f(u + dt*k, t)
// this solver uses a projection assumption, where the
// initial timestep diffusion is assumed throughout
void CustomDummyTimeDependantProblemOperator::ImplicitSolve(
  const mfem::real_t dt, const mfem::Vector &u, mfem::Vector &k)
{
  // Solve the equation:
  //    M*k = -K(u + dt*k) for k = du/dt, if solving for stage-slope
  // or
  //    M*k = -dt*K(k) + M*u for k = u_s, if solving for stage-state
  // where K is linearized by using u from the previous timestep, and
  // the stage-state and slope relation: du/dt = (u_s - u)/dt.
  if (!_T)
  {
    _T = mfem::Add(1.0, _M_mat, dt, _K_mat);
    _current_dt = dt;
    _problem_data.jacobian_solver->SetOperator(*_T);
  }
  MFEM_VERIFY(dt == _current_dt, ""); // SDIRK methods use the same dt

  // Construct current right-hand side for stage state vs. slope solve
  if( ImplicitVarTypeIsState() )
  {
    // k, on return, is the stage value u
    _M_mat.Mult(u, _z);
  }
  else
  {
    // k, on return, is the stage slope du/dt
    _K_mat.Mult(u, _z);
    _z.Neg();
  }
  _problem_data.jacobian_solver->Mult(_z, k);
};

// The function dudt = k = f(u)
// in this case : k = - [M]^-1 [K] u
void CustomDummyTimeDependantProblemOperator::Mult(const mfem::Vector &x, mfem::Vector &y) const
{
  // Get z = [M]*k = -[K]*u
  _K_mat.Mult(x, _z);
  _z.Neg(); // z = -z

  // Get the k = [M]^-1 * z
  M_solver->Mult(_z, y);
};

// Solve a single time-step u(x,t+dt) = G(u(x,t),...)
void CustomDummyTimeDependantProblemOperator::Solve()
{
  // Update the dumm variable
  _dumm_var += 10;

  // Time variables
  auto & dt = _problem.dt();

  //Solve a single step
  _problem_data.ode_solver->Step(_problem_data.true_solution,_problem.time(),dt);


  //Set the GridFunctions from the data
  const std::string _grid_function_name = "var0";
  auto _grid_function = _problem_data.gridfunctions.GetShared(_grid_function_name);
  _grid_function->SetFromTrueDofs(_problem_data.true_solution);
/*
  // Initialise time derivative
  for (const auto & trial_var_name : _trial_var_names)
    gfs.GetRef(tdm.getTimeDerivativeName(trial_var_name)) = gfs.GetRef(trial_var_name);

  // Advance time step of the MFEM problem. Time is also updated here, and
  // _problem_operator->SetTime is called inside the ode_solver->Step method to
  // update the time used by time dependent (function) coefficients.
  _problem_data.ode_solver->Step(*_trial_true_vector, _problem.time(), dt);
  // Synchonise time dependent GridFunctions with updated DoF data.
  SetTrialVariablesFromTrueVectors();

  // Set time derivatives
  for (const auto & trial_var_name : _trial_var_names)
    (gfs.GetRef(tdm.getTimeDerivativeName(trial_var_name)) -= gfs.GetRef(trial_var_name)) /= -dt;
*/
};

namespace Moose::MFEM
{
/**
 * Custom Dummy Operator builder required to build MFEM Problem Operators

 * used by the executioner
 */
class ProblemOperatorBuilderCustomDummy : public ProblemComposerBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = ProblemComposerBase::validParams();
    return params;
  };

  ProblemOperatorBuilderCustomDummy(const InputParameters & parameters)
    : ProblemComposerBase(parameters) {};

  ~ProblemOperatorBuilderCustomDummy() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfemProb) override
  {
    return std::make_shared<CustomDummyTimeDependantProblemOperator>(mfemProb);
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
class MFEMCustomTDProbOperatorTest : public MFEMObjectUnitTest
{
public:
  // The test data
  std::shared_ptr<Moose::MFEM::ProblemComposerBase> probOpBuilder;
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> probOp;

  // The test constructor
  MFEMCustomTDProbOperatorTest() : MFEMObjectUnitTest("MooseUnitApp")
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
    InputParameters prob_Op_params = _factory.getValidParams("ProblemOperatorBuilderCustomDummy");
    _mfem_problem->addMFEMProblemOperator(
        "ProblemOperatorBuilderCustomDummy", "cust_probOp", prob_Op_params);
    probOpBuilder = _mfem_problem->getProblemComposer();
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

TEST_F(MFEMCustomTDProbOperatorTest, TestMFEMCustomOperators)
{
  // Solver problem using the MOOSE-MFEM route
  probOp->Solve();

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
  mfem::ParGridFunction xFunc(&fespace0);
  xFunc = 0.0;

  // Setup the linear forms
  mfem::ConstantCoefficient One(1.0);
  mfem::ParLinearForm bForm(&fespace0);
  bForm.AddDomainIntegrator(new mfem::DomainLFIntegrator(One));
  bForm.Assemble();

  // Setup the bilinear forms
  mfem::ParBilinearForm aForm(&fespace0);
  aForm.AddDomainIntegrator(new mfem::DiffusionIntegrator);
  aForm.Assemble();

  // Form the linear system
  mfem::HypreParMatrix Amat;
  mfem::Vector Bvec, Xvec;
  aForm.FormLinearSystem(boundary_dofs0, xFunc, bForm, Amat, Xvec, Bvec);

  // Set the linear solver and solve
  mfem::MUMPSSolver SolverDir(MPI_COMM_WORLD);
  SolverDir.SetOperator(Amat);
  SolverDir.SetPrintLevel(0);

  // Recover solutions
  aForm.RecoverFEMSolution(Xvec, bForm, xFunc);

  /**
   * Examine test results
   */
  // Check whether the Solver ran with a dummy var
  auto probOp1 = std::static_pointer_cast<CustomDummyTimeDependantProblemOperator>(probOp);
  EXPECT_EQ(probOp1->GetDumVar(), 20);

  // Check number of Vertices in the mesh
  EXPECT_EQ(pmesh.GetNV(), _mfem_mesh_ptr->getMFEMParMesh().GetNV());

  // Check number of DOFs on the boundary
  EXPECT_EQ(pmesh.bdr_attributes.Size(), _mfem_mesh_ptr->getMFEMParMesh().bdr_attributes.Size());

  // Check the error norm of the solution
  const std::string gFuncName = "var0";
  auto gfunc = _mfem_problem->getProblemData().gridfunctions.GetShared(gFuncName);
  xFunc -= *gfunc;
  EXPECT_NEAR(xFunc.Norml2(), 0, 1.0e-7);
};
#endif
