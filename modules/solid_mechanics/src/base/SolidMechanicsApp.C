//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SolidMechanicsApp::validParams()
{
  auto params = MooseApp::validParams();
  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

registerKnownLabel("SolidMechanicsApp");

SolidMechanicsApp::SolidMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  SolidMechanicsApp::registerAll(_factory, _action_factory, _syntax);
}

SolidMechanicsApp::~SolidMechanicsApp() {}

void
SolidMechanicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"SolidMechanicsApp"});
  Registry::registerActionsTo(af, {"SolidMechanicsApp"});
  registerAppDataFilePath("solid_mechanics");

  registerSyntax("EmptyAction", "BCs/CavityPressure");
  registerSyntax("CavityPressureAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressurePPAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressureUOAction", "BCs/CavityPressure/*");

  registerDeprecatedSyntax("LegacyTensorMechanicsAction",
                           "Kernels/TensorMechanics",
                           "The 'Kernels/TensorMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics/QuasiStatic' instead.");
  registerDeprecatedSyntax("LegacyDynamicTensorMechanicsAction",
                           "Kernels/DynamicTensorMechanics",
                           "The 'Kernels/DynamicTensorMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics/Dynamic' instead.");
  // For convenience, since the Kernels/XYZMechanics syntax is still around
  registerDeprecatedSyntax("LegacyTensorMechanicsAction",
                           "Kernels/SolidMechanics",
                           "The 'Kernels/SolidMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics/QuasiStatic' instead.");
  registerDeprecatedSyntax("LegacyDynamicTensorMechanicsAction",
                           "Kernels/DynamicSolidMechanics",
                           "The 'Kernels/DynamicSolidMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics/Dynamic' instead.");
  registerSyntax("PoroMechanicsAction", "Kernels/PoroMechanics");

  registerSyntax("EmptyAction", "BCs/Pressure");
  registerSyntax("PressureAction", "BCs/Pressure/*");
  registerSyntax("EmptyAction", "BCs/InclinedNoDisplacementBC");
  registerSyntax("InclinedNoDisplacementBCAction", "BCs/InclinedNoDisplacementBC/*");
  registerSyntax("EmptyAction", "BCs/CoupledPressure");
  registerSyntax("CoupledPressureAction", "BCs/CoupledPressure/*");

  // Deprecated Modules/TensorMechanics syntax
  registerDeprecatedSyntax("GeneralizedPlaneStrainAction",
                           "Modules/TensorMechanics/GeneralizedPlaneStrain/*",
                           "The 'Modules/TensorMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics' instead.");
  registerDeprecatedSyntax("GlobalStrainAction",
                           "Modules/TensorMechanics/GlobalStrain/*",
                           "The 'Modules/TensorMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics' instead.");
  registerDeprecatedSyntax("CommonSolidMechanicsAction",
                           "Modules/TensorMechanics/Master",
                           "The 'Modules/TensorMechanics/Master' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics/QuasiStatic' instead.");
  registerDeprecatedSyntax(
      "CommonSolidMechanicsAction",
      "Modules/TensorMechanics/DynamicMaster",
      "The 'Modules/TensorMechanics/DynamicMaster' syntax is deprecated. Please use "
      "'Physics/SolidMechanics/Dynamic' instead.");
  registerDeprecatedSyntax("QuasiStaticSolidMechanicsPhysics",
                           "Modules/TensorMechanics/Master/*",
                           "The 'Modules/TensorMechanics/Master' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics/QuasiStatic' instead.");
  registerDeprecatedSyntax(
      "DynamicSolidMechanicsPhysics",
      "Modules/TensorMechanics/DynamicMaster/*",
      "The 'Modules/TensorMechanics/DynamicMaster' syntax is deprecated. Please use "
      "'Physics/SolidMechanics/Dynamic' instead.");

  registerDeprecatedSyntax(
      "CommonLineElementAction",
      "Modules/TensorMechanics/LineElementMaster",
      "The 'Modules/TensorMechanics/LineElementMaster' syntax is deprecated. Please use "
      "'Physics/SolidMechanics/LineElement/QuasiStatic' instead.");
  registerDeprecatedSyntax(
      "LineElementAction",
      "Modules/TensorMechanics/LineElementMaster/*",
      "The 'Modules/TensorMechanics/LineElementMaster' syntax is deprecated. Please use "
      "'Physics/SolidMechanics/LineElement/QuasiStatic' instead.");

  registerDeprecatedSyntax(
      "CommonCohesiveZoneAction",
      "Modules/TensorMechanics/CohesiveZoneMaster",
      "The 'Modules/TensorMechanics/CohesiveZoneMaster' syntax is deprecated. Please use "
      "'Physics/SolidMechanics/CohesiveZone' instead.");
  registerDeprecatedSyntax(
      "CohesiveZoneAction",
      "Modules/TensorMechanics/CohesiveZoneMaster/*",
      "The 'Modules/TensorMechanics/CohesiveZoneMaster' syntax is deprecated. Please use "
      "'Physics/SolidMechanics/CohesiveZone' instead.");

  registerDeprecatedSyntax("EmptyAction",
                           "Modules/TensorMechanics/MaterialVectorBodyForce",
                           "The 'Modules/TensorMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics' instead.");
  registerDeprecatedSyntax("MaterialVectorBodyForceAction",
                           "Modules/TensorMechanics/MaterialVectorBodyForce/*",
                           "The 'Modules/TensorMechanics' syntax is deprecated. Please use "
                           "'Physics/SolidMechanics' instead.");

  // New Physics syntax
  registerSyntax("GeneralizedPlaneStrainAction", "Physics/SolidMechanics/GeneralizedPlaneStrain/*");
  registerSyntax("GlobalStrainAction", "Physics/SolidMechanics/GlobalStrain/*");
  registerSyntax("CommonSolidMechanicsAction", "Physics/SolidMechanics/QuasiStatic");
  registerSyntax("CommonSolidMechanicsAction", "Physics/SolidMechanics/Dynamic");
  registerSyntax("QuasiStaticSolidMechanicsPhysics", "Physics/SolidMechanics/QuasiStatic/*");
  registerSyntax("DynamicSolidMechanicsPhysics", "Physics/SolidMechanics/Dynamic/*");

  registerSyntax("CommonLineElementAction", "Physics/SolidMechanics/LineElement/QuasiStatic");
  registerSyntax("LineElementAction", "Physics/SolidMechanics/LineElement/QuasiStatic/*");

  registerSyntax("CommonCohesiveZoneAction", "Physics/SolidMechanics/CohesiveZone");
  registerSyntax("CohesiveZoneAction", "Physics/SolidMechanics/CohesiveZone/*");

  registerSyntax("EmptyAction", "Physics/SolidMechanics/MaterialVectorBodyForce");
  registerSyntax("MaterialVectorBodyForceAction",
                 "Physics/SolidMechanics/MaterialVectorBodyForce/*");

  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_user_object");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_variable");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_kernel");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_vector_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_material");

  registerTask("validate_coordinate_systems", /*is_required=*/false);
  addTaskDependency("validate_coordinate_systems", "create_problem_complete");
  addTaskDependency("setup_postprocessor_data", "validate_coordinate_systems");
}

void
SolidMechanicsApp::registerApps()
{
  registerApp(SolidMechanicsApp);
}

extern "C" void
SolidMechanicsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidMechanicsApp::registerAll(f, af, s);
}
extern "C" void
SolidMechanicsApp_registerApps()
{
  SolidMechanicsApp::registerApps();
}
