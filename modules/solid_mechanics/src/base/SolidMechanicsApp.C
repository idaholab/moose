/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "AbaqusCreepMaterial.h"
#include "AbaqusUmatMaterial.h"
#include "AdaptiveTransient.h"
#include "CLSHPlasticMaterial.h"
#include "CLSHPlasticModel.h"
#include "CombinedCreepPlasticity.h"
#include "DashpotBC.h"
#include "Elastic.h"
#include "ElasticModel.h"
#include "ElasticEnergyAux.h"
#include "ElementsOnLineAux.h"
#include "Gravity.h"
#include "HomogenizationKernel.h"
#include "HomogenizedElasticConstants.h"
#include "HomogenizationHeatConduction.h"
#include "HomogenizedThermalConductivity.h"
#include "IsotropicPlasticity.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearGeneralAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "MacroElastic.h"
#include "Mass.h"
#include "JIntegral.h"
#include "CrackFrontData.h"
#include "CrackFrontDefinition.h"
#include "InteractionIntegral.h"
#include "InteractionIntegralAuxFields.h"
#include "MaterialSymmElasticityTensorAux.h"
#include "MaterialTensorAux.h"
#include "MaterialTensorOnLine.h"
#include "MaterialVectorAux.h"
#include "AccumulateAux.h"
#include "NewmarkAccelAux.h"
#include "NewmarkVelAux.h"
#include "DomainIntegralQFunction.h"
#include "PLC_LSH.h"
#include "PowerLawCreep.h"
#include "PowerLawCreepModel.h"
#include "CavityPressureAction.h"
#include "CavityPressurePostprocessor.h"
#include "LineMaterialSymmTensorSampler.h"
#include "CavityPressurePPAction.h"
#include "CavityPressureUserObject.h"
#include "CavityPressureUOAction.h"
#include "PresetVelocity.h"
#include "Pressure.h"
#include "PressureAction.h"
#include "DisplacementAboutAxis.h"
#include "DisplacementAboutAxisAction.h"
#include "InteractionIntegralBenchmarkBC.h"
#include "TorqueReaction.h"
#include "CrackDataSampler.h"
#include "SolidMechanicsAction.h"
#include "DomainIntegralAction.h"
#include "SolidMechInertialForce.h"
#include "SolidMechImplicitEuler.h"
#include "SolidModel.h"
#include "StressDivergence.h"
#include "StressDivergenceRZ.h"
#include "StressDivergenceRSpherical.h"
#include "StressDivergenceTruss.h"
#include "TrussMaterial.h"
#include "RateDepSmearCrackModel.h"
#include "RateDepSmearIsoCrackModel.h"


template<>
InputParameters validParams<SolidMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = true;
  params.set<bool>("use_legacy_uo_aux_computation") = false;

  return params;
}

SolidMechanicsApp::SolidMechanicsApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  SolidMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntax(_syntax, _action_factory);
}

SolidMechanicsApp::~SolidMechanicsApp()
{
}

void
SolidMechanicsApp::registerApps()
{
  registerApp(SolidMechanicsApp);
}

void
SolidMechanicsApp::registerObjects(Factory & factory)
{
  registerAux(ElasticEnergyAux);
  registerAux(MaterialSymmElasticityTensorAux);
  registerAux(MaterialTensorAux);
  registerAux(MaterialVectorAux);
  registerAux(AccumulateAux);
  registerAux(NewmarkAccelAux);
  registerAux(NewmarkVelAux);
  registerAux(DomainIntegralQFunction);
  registerAux(ElementsOnLineAux);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);
  registerBoundaryCondition(DisplacementAboutAxis);
  registerBoundaryCondition(InteractionIntegralBenchmarkBC);

  registerExecutioner(AdaptiveTransient);

  registerMaterial(AbaqusCreepMaterial);
  registerMaterial(AbaqusUmatMaterial);
  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(CLSHPlasticModel);
  registerMaterial(CombinedCreepPlasticity);
  registerMaterial(Elastic);
  registerMaterial(ElasticModel);
  registerMaterial(InteractionIntegralAuxFields);
  registerMaterial(IsotropicPlasticity);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearGeneralAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(LinearStrainHardening);
  registerMaterial(MacroElastic);
  registerMaterial(PLC_LSH);
  registerMaterial(PowerLawCreep);
  registerMaterial(PowerLawCreepModel);
  registerMaterial(SolidModel);
  registerMaterial(TrussMaterial);
  registerMaterial(RateDepSmearCrackModel);
  registerMaterial(RateDepSmearIsoCrackModel);

  registerKernel(Gravity);
  registerKernel(HomogenizationKernel);
  registerKernel(SolidMechImplicitEuler);
  registerKernel(SolidMechInertialForce);
  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);
  registerKernel(StressDivergenceRSpherical);
  registerKernel(StressDivergenceTruss);
  registerKernel(HomogenizationHeatConduction);

  registerPostprocessor(HomogenizedThermalConductivity);
  registerPostprocessor(HomogenizedElasticConstants);
  registerPostprocessor(Mass);
  registerPostprocessor(JIntegral);
  registerPostprocessor(CrackFrontData);
  registerPostprocessor(InteractionIntegral);
  registerPostprocessor(CavityPressurePostprocessor);
  registerPostprocessor(TorqueReaction);

  registerVectorPostprocessor(CrackDataSampler);
  registerVectorPostprocessor(LineMaterialSymmTensorSampler);

  registerUserObject(MaterialTensorOnLine);
  registerUserObject(CavityPressureUserObject);
  registerUserObject(CrackFrontDefinition);
}

void
SolidMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("EmptyAction", "BCs/CavityPressure");
  syntax.registerActionSyntax("CavityPressureAction", "BCs/CavityPressure/*");
  syntax.registerActionSyntax("CavityPressurePPAction", "BCs/CavityPressure/*");
  syntax.registerActionSyntax("CavityPressureUOAction", "BCs/CavityPressure/*");

  syntax.registerActionSyntax("EmptyAction", "BCs/Pressure");
  syntax.registerActionSyntax("PressureAction", "BCs/Pressure/*");

  syntax.registerActionSyntax("EmptyAction", "BCs/DisplacementAboutAxis");
  syntax.registerActionSyntax("DisplacementAboutAxisAction", "BCs/DisplacementAboutAxis/*");

  syntax.registerActionSyntax("SolidMechanicsAction", "SolidMechanics/*");

  syntax.registerActionSyntax("DomainIntegralAction", "DomainIntegral","add_user_object");
  syntax.registerActionSyntax("DomainIntegralAction", "DomainIntegral","add_aux_variable");
  syntax.registerActionSyntax("DomainIntegralAction", "DomainIntegral","add_aux_kernel");
  syntax.registerActionSyntax("DomainIntegralAction", "DomainIntegral","add_postprocessor");
  syntax.registerActionSyntax("DomainIntegralAction", "DomainIntegral","add_vector_postprocessor");
  syntax.registerActionSyntax("DomainIntegralAction", "DomainIntegral","add_material");

  registerAction(PressureAction, "add_bc");
  registerAction(DisplacementAboutAxisAction, "add_bc");
  registerAction(CavityPressureAction, "add_bc");
  registerAction(CavityPressurePPAction, "add_postprocessor");
  registerAction(CavityPressureUOAction, "add_user_object");
  registerAction(SolidMechanicsAction, "add_kernel");
  registerAction(DomainIntegralAction, "add_user_object");
  registerAction(DomainIntegralAction, "add_aux_variable");
  registerAction(DomainIntegralAction, "add_aux_kernel");
  registerAction(DomainIntegralAction, "add_postprocessor");
  registerAction(DomainIntegralAction, "add_material");
}
