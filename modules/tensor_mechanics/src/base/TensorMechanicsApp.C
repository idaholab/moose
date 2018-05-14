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

template <>
InputParameters
validParams<TensorMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("TensorMechanicsApp");

TensorMechanicsApp::TensorMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  TensorMechanicsApp::registerExecFlags(_factory);
}

TensorMechanicsApp::~TensorMechanicsApp() {}

// External entry point for dynamic application loading
extern "C" void
TensorMechanicsApp_registerApps()
{
  TensorMechanicsApp::registerApps();
}
void
TensorMechanicsApp::registerApps()
{
  registerApp(TensorMechanicsApp);
}

// External entry point for dynamic object registration
extern "C" void
TensorMechanicsApp__registerObjects(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}
void
TensorMechanicsApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"TensorMechanicsApp"});
}

// External entry point for dynamic syntax association
extern "C" void
TensorMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}
void
TensorMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"TensorMechanicsApp"});

  registerSyntax("EmptyAction", "BCs/CavityPressure");
  registerSyntax("CavityPressureAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressurePPAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressureUOAction", "BCs/CavityPressure/*");

  registerSyntax("LegacyTensorMechanicsAction", "Kernels/TensorMechanics");
  registerSyntax("DynamicTensorMechanicsAction", "Kernels/DynamicTensorMechanics");
  registerSyntax("PoroMechanicsAction", "Kernels/PoroMechanics");

  registerSyntax("EmptyAction", "BCs/Pressure");
  registerSyntax("PressureAction", "BCs/Pressure/*");
  registerSyntax("EmptyAction", "BCs/CoupledPressure");
  registerSyntax("CoupledPressureAction", "BCs/CoupledPressure/*");

  registerSyntax("GeneralizedPlaneStrainAction",
                 "Modules/TensorMechanics/GeneralizedPlaneStrain/*");
  registerSyntax("GlobalStrainAction", "Modules/TensorMechanics/GlobalStrain/*");
  registerSyntax("CommonTensorMechanicsAction", "Modules/TensorMechanics/Master");
  registerSyntax("TensorMechanicsAction", "Modules/TensorMechanics/Master/*");

  registerSyntax("CommonLineElementAction", "Modules/TensorMechanics/LineElementMaster");
  registerSyntax("LineElementAction", "Modules/TensorMechanics/LineElementMaster/*");

  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_user_object");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_variable");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_kernel");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_vector_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_material");

  registerTask("validate_coordinate_systems", /*is_required=*/false);
  addTaskDependency("validate_coordinate_systems", "create_problem");
}

// External entry point for dynamic execute flag registration
extern "C" void
TensorMechanicsApp__registerExecFlags(Factory & factory)
{
  TensorMechanicsApp::registerExecFlags(factory);
}
void
TensorMechanicsApp::registerExecFlags(Factory & /*factory*/)
{
}
