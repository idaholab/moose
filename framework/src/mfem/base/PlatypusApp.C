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
  registerMooseObjectTask("add_mfem_formulation", MFEMFormulation, true);
  registerSyntaxTask("AddFormulationAction", "Formulation", "add_mfem_formulation");
  addTaskDependency("add_mfem_formulation", "init_mesh");
  addTaskDependency("add_variable", "add_mfem_formulation");
  addTaskDependency("add_aux_variable", "add_mfem_formulation");
  addTaskDependency("add_elemental_field_variable", "add_mfem_formulation");
  addTaskDependency("add_kernel", "add_mfem_formulation");

  // add MFEM auxkernel base
  appendMooseObjectTask("add_aux_kernel", MFEMAuxKernel);

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

  // add sources
  registerMooseObjectTask("add_mfem_sources", MFEMSource, false);
  registerSyntaxTask("AddSourceAction", "Sources/*", "add_mfem_sources");
  addTaskDependency("add_mfem_sources", "add_material");
  addTaskDependency("add_mfem_sources", "add_variable");
  addTaskDependency("add_mfem_sources", "add_aux_variable");

  // add FESpaces
  registerMooseObjectTask("add_mfem_fespaces", MFEMFESpace, false);
  registerSyntaxTask("AddFESpaceAction", "FESpaces/*", "add_mfem_fespaces");
  addTaskDependency("add_variable", "add_mfem_fespaces");
  addTaskDependency("add_aux_variable", "add_mfem_fespaces");
  addTaskDependency("add_elemental_field_variable", "add_mfem_fespaces");
  addTaskDependency("add_kernel", "add_mfem_fespaces");
  addTaskDependency("add_mfem_sources", "add_mfem_fespaces");
}

void
PlatypusApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
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
