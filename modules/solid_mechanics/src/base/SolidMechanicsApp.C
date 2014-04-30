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
#include "CrackFrontDefinition.h"
#include "MaterialSymmElasticityTensorAux.h"
#include "MaterialTensorAux.h"
#include "MaterialTensorOnLine.h"
#include "MaterialVectorAux.h"
#include "AccumulateAux.h"
#include "NewmarkAccelAux.h"
#include "NewmarkVelAux.h"
#include "qFunctionJIntegral.h"
#include "PLC_LSH.h"
#include "PowerLawCreep.h"
#include "PowerLawCreepModel.h"
#include "CavityPressureAction.h"
#include "CavityPressurePostprocessor.h"
#include "CavityPressurePPAction.h"
#include "CavityPressureUserObject.h"
#include "CavityPressureUOAction.h"
#include "PresetVelocity.h"
#include "Pressure.h"
#include "PressureAction.h"
#include "SolidMechanicsAction.h"
#include "JIntegralAction.h"
#include "SolidMechInertialForce.h"
#include "SolidMechImplicitEuler.h"
#include "SolidModel.h"
#include "StressDivergence.h"
#include "StressDivergenceRZ.h"
#include "StressDivergenceRSpherical.h"
#include "StressDivergenceTruss.h"
#include "TrussMaterial.h"


template<>
InputParameters validParams<SolidMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
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
  registerAux(qFunctionJIntegral);
  registerAux(ElementsOnLineAux);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);

  registerExecutioner(AdaptiveTransient);

  registerMaterial(AbaqusCreepMaterial);
  registerMaterial(AbaqusUmatMaterial);
  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(CLSHPlasticModel);
  registerMaterial(CombinedCreepPlasticity);
  registerMaterial(Elastic);
  registerMaterial(ElasticModel);
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
  registerPostprocessor(CavityPressurePostprocessor);

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

  syntax.registerActionSyntax("SolidMechanicsAction", "SolidMechanics/*");

  syntax.registerActionSyntax("JIntegralAction", "JIntegral","add_user_object");
  syntax.registerActionSyntax("JIntegralAction", "JIntegral","add_aux_variable");
  syntax.registerActionSyntax("JIntegralAction", "JIntegral","add_aux_kernel");
  syntax.registerActionSyntax("JIntegralAction", "JIntegral","add_postprocessor");

  registerAction(PressureAction, "add_bc");
  registerAction(CavityPressureAction, "add_bc");
  registerAction(CavityPressurePPAction, "add_postprocessor");
  registerAction(CavityPressureUOAction, "add_user_object");
  registerAction(SolidMechanicsAction, "add_kernel");
  registerAction(JIntegralAction, "add_user_object");
  registerAction(JIntegralAction, "add_aux_variable");
  registerAction(JIntegralAction, "add_aux_kernel");
  registerAction(JIntegralAction, "add_postprocessor");
}
