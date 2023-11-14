//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
TensorMechanicsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("TensorMechanicsApp");

TensorMechanicsApp::TensorMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  TensorMechanicsApp::registerAll(_factory, _action_factory, _syntax);
}

TensorMechanicsApp::~TensorMechanicsApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntax("EmptyAction", "BCs/CavityPressure");
  registerSyntax("CavityPressureAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressurePPAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressureUOAction", "BCs/CavityPressure/*");

  registerSyntax("LegacyTensorMechanicsAction", "Kernels/TensorMechanics");
  registerSyntax("LegacyDynamicTensorMechanicsAction", "Kernels/DynamicTensorMechanics");
  registerSyntax("PoroMechanicsAction", "Kernels/PoroMechanics");

  registerSyntax("EmptyAction", "BCs/Pressure");
  registerSyntax("PressureAction", "BCs/Pressure/*");
  registerSyntax("EmptyAction", "BCs/InclinedNoDisplacementBC");
  registerSyntax("InclinedNoDisplacementBCAction", "BCs/InclinedNoDisplacementBC/*");
  registerSyntax("EmptyAction", "BCs/CoupledPressure");
  registerSyntax("CoupledPressureAction", "BCs/CoupledPressure/*");

  registerSyntax("GeneralizedPlaneStrainAction",
                 "Modules/TensorMechanics/GeneralizedPlaneStrain/*");
  registerSyntax("GlobalStrainAction", "Modules/TensorMechanics/GlobalStrain/*");
  registerSyntax("CommonTensorMechanicsAction", "Modules/TensorMechanics/Master");
  registerSyntax("CommonTensorMechanicsAction", "Modules/TensorMechanics/DynamicMaster");
  registerSyntax("TensorMechanicsAction", "Modules/TensorMechanics/Master/*");
  registerSyntax("DynamicTensorMechanicsAction", "Modules/TensorMechanics/DynamicMaster/*");

  registerSyntax("CommonLineElementAction", "Modules/TensorMechanics/LineElementMaster");
  registerSyntax("LineElementAction", "Modules/TensorMechanics/LineElementMaster/*");

  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_user_object");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_variable");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_kernel");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_vector_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_material");

  registerSyntax("CommonCohesiveZoneAction", "Modules/TensorMechanics/CohesiveZoneMaster");
  registerSyntax("CohesiveZoneAction", "Modules/TensorMechanics/CohesiveZoneMaster/*");

  registerSyntax("EmptyAction", "Modules/TensorMechanics/MaterialVectorBodyForce");
  registerSyntax("MaterialVectorBodyForceAction",
                 "Modules/TensorMechanics/MaterialVectorBodyForce/*");

  registerTask("validate_coordinate_systems", /*is_required=*/false);
  addTaskDependency("validate_coordinate_systems", "create_problem_complete");
  addTaskDependency("setup_postprocessor_data", "validate_coordinate_systems");
}

void
TensorMechanicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"TensorMechanicsApp"});
  Registry::registerActionsTo(af, {"TensorMechanicsApp"});
  associateSyntaxInner(s, af);
  registerDataFilePath();
}

void
TensorMechanicsApp::registerApps()
{
  registerApp(TensorMechanicsApp);
}

void
TensorMechanicsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"TensorMechanicsApp"});
}

void
TensorMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"TensorMechanicsApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
TensorMechanicsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
TensorMechanicsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  TensorMechanicsApp::registerAll(f, af, s);
}
extern "C" void
TensorMechanicsApp_registerApps()
{
  TensorMechanicsApp::registerApps();
}
