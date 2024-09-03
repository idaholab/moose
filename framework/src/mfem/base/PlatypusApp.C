#include "PlatypusApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
PlatypusApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

PlatypusApp::PlatypusApp(InputParameters parameters) : MooseApp(parameters)
{
  PlatypusApp::registerAll(_factory, _action_factory, _syntax);
}

PlatypusApp::~PlatypusApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  // add base formulation
  registerTask("add_mfem_problem_builder", true);
  addTaskDependency("add_mfem_problem_builder", "init_mesh");
  addTaskDependency("add_variable", "add_mfem_problem_builder");
  addTaskDependency("add_aux_variable", "add_mfem_problem_builder");
  addTaskDependency("add_elemental_field_variable", "add_mfem_problem_builder");
  addTaskDependency("add_kernel", "add_mfem_problem_builder");

  // add coefficients
  registerMooseObjectTask("add_mfem_coefficients", MFEMCoefficient, false);
  registerSyntaxTask("AddCoefficientAction", "Coefficients/*", "add_mfem_coefficients");
  addTaskDependency("add_material", "add_mfem_coefficients");
  addTaskDependency("add_mfem_coefficients", "add_variable");
  addTaskDependency("add_mfem_coefficients", "add_aux_variable");
  addTaskDependency("add_mfem_coefficients", "add_ic");

  // add vector coefficients
  registerMooseObjectTask("add_mfem_vector_coefficients", MFEMVectorCoefficient, false);
  registerSyntaxTask(
      "AddVectorCoefficientAction", "VectorCoefficients/*", "add_mfem_vector_coefficients");
  addTaskDependency("add_material", "add_mfem_vector_coefficients");

  // add FESpaces
  registerMooseObjectTask("add_mfem_fespaces", MFEMFESpace, false);
  appendMooseObjectTask("add_mfem_fespaces", MFEMFECollection);
  registerSyntaxTask("AddFESpaceAction", "FESpaces/*", "add_mfem_fespaces");
  addTaskDependency("add_variable", "add_mfem_fespaces");
  addTaskDependency("add_aux_variable", "add_mfem_fespaces");
  addTaskDependency("add_elemental_field_variable", "add_mfem_fespaces");
  addTaskDependency("add_kernel", "add_mfem_fespaces");

  // add preconditioning.
  registerMooseObjectTask("add_mfem_preconditioner", MFEMSolverBase, false);
  registerSyntaxTask("AddMFEMPreconditionerAction", "Preconditioner/*", "add_mfem_preconditioner");
  addTaskDependency("add_mfem_preconditioner", "add_mfem_problem_builder");
  addTaskDependency("add_mfem_preconditioner", "add_variable");

  // add solver.
  registerMooseObjectTask("add_mfem_solver", MFEMSolverBase, true);
  registerSyntaxTask("AddMFEMSolverAction", "Solver", "add_mfem_solver");
  addTaskDependency("add_mfem_solver", "add_mfem_preconditioner");
  addTaskDependency("add_mfem_solver", "add_mfem_problem_builder");
}

void
PlatypusApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAllObjects<PlatypusApp>(f, af, s);
  Registry::registerObjectsTo(f, {"PlatypusApp"});
  Registry::registerActionsTo(af, {"PlatypusApp"});
  /* register custom execute flags, action syntax, etc. here */
  associateSyntaxInner(s, af);
}

void
PlatypusApp::registerApps()
{
  registerApp(PlatypusApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
PlatypusApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PlatypusApp::registerAll(f, af, s);
}
extern "C" void
PlatypusApp__registerApps()
{
  PlatypusApp::registerApps();
}
