#include "PorousFlowApp.h"
#include "Moose.h"
#include "TensorMechanicsApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// Kernels
#include "PorousFlowMassTimeDerivative.h"
#include "PorousFlowAdvectiveFlux.h"
#include "PorousFlowEffectiveStressCoupling.h"
#include "PorousFlowMassVolumetricExpansion.h"

// UserObjects
#include "PorousFlowDictator.h"

// Materials
#include "PorousFlowVariableBase.h"
#include "PorousFlowMaterial1PhaseP.h"
#include "PorousFlowMaterial1PhaseP_VG.h"
#include "PorousFlowMaterial1PhaseMD_Gaussian.h"
#include "PorousFlowMaterial2PhasePS_VG.h"
#include "PorousFlowMaterial2PhasePP_VG.h"
#include "PorousFlowMaterialDensityConstBulk.h"
#include "PorousFlowMaterialPorosityConst.h"
#include "PorousFlowPorosityHM.h"
#include "PorousFlowMaterialMassFractionBuilder.h"
#include "PorousFlowMaterialPermeabilityConst.h"
#include "PorousFlowMaterialRelativePermeabilityBase.h"
#include "PorousFlowMaterialRelativePermeabilityConstant.h"
#include "PorousFlowMaterialRelativePermeabilityLinear.h"
#include "PorousFlowMaterialRelativePermeabilityCorey.h"
#include "PorousFlowMaterialCapillaryPressureBase.h"
#include "PorousFlowMaterialCapillaryPressureConstant.h"
#include "PorousFlowMaterialCapillaryPressureLinear.h"
#include "PorousFlowMaterialCapillaryPressureVG.h"
#include "PorousFlowMaterialCapillaryPressureVGP.h"
#include "PorousFlowMaterialViscosityConst.h"
#include "PorousFlowMaterialJoiner.h"
#include "PorousFlowMaterialJoinerOld.h"
#include "PorousFlowMaterialEffectiveFluidPressure.h"
#include "PorousFlowMaterialFluidPropertiesBase.h"
#include "PorousFlowMaterialIdealGas.h"
#include "PorousFlowMaterialMethane.h"
#include "PorousFlowMaterialWater.h"
#include "PorousFlowMaterialBrine.h"
#include "PorousFlowMaterialSimpleCO2.h"
#include "PorousFlowVolumetricStrain.h"

// Postprocessors
#include "PorousFlowFluidMass.h"


//dirac kernel



#include "PorousFlowTimeLimitedConstantPointSource.h"
#include "PorousFlowSquarePulsePointSource.h"


template<>
InputParameters validParams<PorousFlowApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

PorousFlowApp::PorousFlowApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
  PorousFlowApp::associateSyntax(_syntax, _action_factory);
}

PorousFlowApp::~PorousFlowApp()
{
}

// External entry point for dynamic application loading
extern "C" void PorousFlowApp__registerApps() { PorousFlowApp::registerApps(); }
void
PorousFlowApp::registerApps()
{
  registerApp(PorousFlowApp);
}

// External entry point for dynamic object registration
extern "C" void PorousFlowApp__registerObjects(Factory & factory) { PorousFlowApp::registerObjects(factory); }
void
PorousFlowApp::registerObjects(Factory & factory)
{
  // Kernels
  registerKernel(PorousFlowMassTimeDerivative);
  registerKernel(PorousFlowAdvectiveFlux);
  registerKernel(PorousFlowEffectiveStressCoupling);
  registerKernel(PorousFlowMassVolumetricExpansion);

  // UserObjects
  registerUserObject(PorousFlowDictator);

  // Materials
  registerMaterial(PorousFlowVariableBase);
  registerMaterial(PorousFlowMaterial1PhaseP);
  registerMaterial(PorousFlowMaterial1PhaseP_VG);
  registerMaterial(PorousFlowMaterial1PhaseMD_Gaussian);
  registerMaterial(PorousFlowMaterial2PhasePS_VG);
  registerMaterial(PorousFlowMaterial2PhasePP_VG);
  registerMaterial(PorousFlowMaterialDensityConstBulk);
  registerMaterial(PorousFlowMaterialPorosityConst);
  registerMaterial(PorousFlowPorosityHM);
  registerMaterial(PorousFlowMaterialMassFractionBuilder);
  registerMaterial(PorousFlowMaterialPermeabilityConst);
  registerMaterial(PorousFlowMaterialCapillaryPressureBase);
  registerMaterial(PorousFlowMaterialCapillaryPressureConstant);
  registerMaterial(PorousFlowMaterialCapillaryPressureLinear);
  registerMaterial(PorousFlowMaterialCapillaryPressureVG);
  registerMaterial(PorousFlowMaterialCapillaryPressureVGP);
  registerMaterial(PorousFlowMaterialRelativePermeabilityBase);
  registerMaterial(PorousFlowMaterialRelativePermeabilityConstant);
  registerMaterial(PorousFlowMaterialRelativePermeabilityLinear);
  registerMaterial(PorousFlowMaterialRelativePermeabilityCorey);
  registerMaterial(PorousFlowMaterialViscosityConst);
  registerMaterial(PorousFlowMaterialJoiner);
  registerMaterial(PorousFlowMaterialJoinerOld);
  registerMaterial(PorousFlowMaterialEffectiveFluidPressure);
  registerMaterial(PorousFlowMaterialFluidPropertiesBase);
  registerMaterial(PorousFlowMaterialIdealGas);
  registerMaterial(PorousFlowMaterialMethane);
  registerMaterial(PorousFlowMaterialWater);
  registerMaterial(PorousFlowMaterialBrine);
  registerMaterial(PorousFlowMaterialSimpleCO2);
  registerMaterial(PorousFlowVolumetricStrain);

  // Postprocessors
  registerPostprocessor(PorousFlowFluidMass);


  //Dirac Kernel



  registerDiracKernel(PorousFlowTimeLimitedConstantPointSource);
  registerDiracKernel(PorousFlowSquarePulsePointSource);
}

// External entry point for dynamic syntax association
extern "C" void PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PorousFlowApp::associateSyntax(syntax, action_factory); }
void
PorousFlowApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
