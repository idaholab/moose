/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "AbaqusCreepMaterial.h"
#include "AbaqusUmatMaterial.h"
#include "CLSHPlasticMaterial.h"
#include "CLSHPlasticModel.h"
#include "CombinedCreepPlasticity.h"
#include "Elastic.h"
#include "ElasticModel.h"
#include "HomogenizationKernel.h"
#include "HomogenizedElasticConstants.h"
#include "IsotropicPlasticity.h"
#include "IsotropicPowerLawHardening.h"
#include "IsotropicTempDepHardening.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearGeneralAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "MacroElastic.h"
#include "JIntegral.h"
#include "CrackFrontData.h"
#include "CrackFrontDefinition.h"
#include "InteractionIntegral.h"
#include "InteractionIntegralAuxFields.h"
#include "MixedModeEquivalentK.h"
#include "MaterialSymmElasticityTensorAux.h"
#include "MaterialTensorAux.h"
#include "DomainIntegralQFunction.h"
#include "DomainIntegralTopologicalQFunction.h"
#include "PLC_LSH.h"
#include "PowerLawCreep.h"
#include "PowerLawCreepModel.h"
#include "InteractionIntegralBenchmarkBC.h"
#include "MaterialTensorIntegralSM.h"
#include "CrackDataSampler.h"
#include "LineMaterialSymmTensorSampler.h"
#include "SolidMechanicsAction.h"
#include "DomainIntegralAction.h"
#include "SolidMechImplicitEuler.h"
#include "SolidModel.h"
#include "StressDivergence.h"
#include "OutOfPlaneStress.h"
#include "StressDivergenceRZ.h"
#include "StressDivergenceRSpherical.h"
#include "RateDepSmearCrackModel.h"
#include "RateDepSmearIsoCrackModel.h"

template <>
InputParameters
validParams<SolidMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

SolidMechanicsApp::SolidMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);
  SolidMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags();
  SolidMechanicsApp::registerExecFlags();
}

SolidMechanicsApp::~SolidMechanicsApp() {}

// External entry point for dynamic application loading
extern "C" void
SolidMechanicsApp__registerApps()
{
  SolidMechanicsApp::registerApps();
}
void
SolidMechanicsApp::registerApps()
{
  registerApp(SolidMechanicsApp);
}

// External entry point for dynamic object registration
extern "C" void
SolidMechanicsApp__registerObjects(Factory & factory)
{
  SolidMechanicsApp::registerObjects(factory);
}
void
SolidMechanicsApp::registerObjects(Factory & factory)
{
  registerAux(MaterialSymmElasticityTensorAux);
  registerAux(MaterialTensorAux);
  registerAux(DomainIntegralQFunction);
  registerAux(DomainIntegralTopologicalQFunction);

  registerBoundaryCondition(InteractionIntegralBenchmarkBC);

  registerMaterial(AbaqusCreepMaterial);
  registerMaterial(AbaqusUmatMaterial);
  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(CLSHPlasticModel);
  registerMaterial(CombinedCreepPlasticity);
  registerMaterial(Elastic);
  registerMaterial(ElasticModel);
  registerMaterial(InteractionIntegralAuxFields);
  registerMaterial(IsotropicPlasticity);
  registerMaterial(IsotropicPowerLawHardening);
  registerMaterial(IsotropicTempDepHardening);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearGeneralAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(LinearStrainHardening);
  registerMaterial(MacroElastic);
  registerMaterial(PLC_LSH);
  registerMaterial(PowerLawCreep);
  registerMaterial(PowerLawCreepModel);
  registerMaterial(SolidModel);
  registerMaterial(RateDepSmearCrackModel);
  registerMaterial(RateDepSmearIsoCrackModel);

  registerKernel(HomogenizationKernel);
  registerKernel(SolidMechImplicitEuler);
  registerKernel(StressDivergence);
  registerKernel(OutOfPlaneStress);
  registerKernel(StressDivergenceRZ);
  registerKernel(StressDivergenceRSpherical);

  registerPostprocessor(HomogenizedElasticConstants);
  registerPostprocessor(JIntegral);
  registerPostprocessor(CrackFrontData);
  registerPostprocessor(InteractionIntegral);
  registerPostprocessor(MaterialTensorIntegralSM);
  registerPostprocessor(MixedModeEquivalentK);

  registerVectorPostprocessor(CrackDataSampler);
  registerVectorPostprocessor(LineMaterialSymmTensorSampler);

  registerUserObject(CrackFrontDefinition);
}

// External entry point for dynamic syntax association
extern "C" void
SolidMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
}
void
SolidMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntax("SolidMechanicsAction", "SolidMechanics/*");

  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_user_object");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_variable");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_kernel");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_vector_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_material");

  registerAction(SolidMechanicsAction, "add_kernel");
  registerAction(DomainIntegralAction, "add_user_object");
  registerAction(DomainIntegralAction, "add_aux_variable");
  registerAction(DomainIntegralAction, "add_aux_kernel");
  registerAction(DomainIntegralAction, "add_postprocessor");
  registerAction(DomainIntegralAction, "add_material");
}

void
SolidMechanicsApp::registerExecFlags()
{
}
