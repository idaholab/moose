#include "ActionFactory.h"
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
  registerTask("add_mfem_problem_operator", true);
  addTaskDependency("add_mfem_problem_operator", "init_mesh");
  addTaskDependency("add_variable", "add_mfem_problem_operator");
  addTaskDependency("add_aux_variable", "add_mfem_problem_operator");
  addTaskDependency("add_elemental_field_variable", "add_mfem_problem_operator");
  addTaskDependency("add_kernel", "add_mfem_problem_operator");

  // add FESpaces
  registerMooseObjectTask("add_mfem_fespaces", MFEMFESpace, false);
  appendMooseObjectTask("add_mfem_fespaces", MFEMFECollection);
  registerSyntaxTask("AddFESpaceAction", "FESpaces/*", "add_mfem_fespaces");
  addTaskDependency("add_variable", "add_mfem_fespaces");
  addTaskDependency("add_aux_variable", "add_mfem_fespaces");
  addTaskDependency("add_elemental_field_variable", "add_mfem_fespaces");
  addTaskDependency("add_kernel", "add_mfem_fespaces");

  // set mesh FE space
  registerTask("set_mesh_fe_space", true);
  addTaskDependency("set_mesh_fe_space", "add_variable");
  addTaskDependency("set_mesh_fe_space", "init_mesh");

  // add preconditioning.
  registerMooseObjectTask("add_mfem_preconditioner", MFEMSolverBase, false);
  registerSyntaxTask("AddMFEMPreconditionerAction", "Preconditioner/*", "add_mfem_preconditioner");
  addTaskDependency("add_mfem_preconditioner", "add_mfem_problem_operator");
  addTaskDependency("add_mfem_preconditioner", "add_variable");

  // add solver.
  registerMooseObjectTask("add_mfem_solver", MFEMSolverBase, true);
  registerSyntaxTask("AddMFEMSolverAction", "Solver", "add_mfem_solver");
  addTaskDependency("add_mfem_solver", "add_mfem_preconditioner");
  addTaskDependency("add_mfem_solver", "add_mfem_problem_operator");
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
