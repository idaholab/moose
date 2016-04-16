#include "PorousFlowApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// Kernels
#include "PorousFlowMassTimeDerivative.h"
#include "PorousFlowAdvectiveFlux.h"
#include "PorousFlowTestKernel.h"

// UserObjects
#include "PorousFlowDictator.h"

// Materials
#include "PorousFlowMaterial1PhaseP_VG.h"
#include "PorousFlowMaterial2PhasePS.h"
#include "PorousFlowMaterial2PhasePP_VG.h"
#include "PorousFlowMaterialDensityConstBulk.h"
#include "PorousFlowMaterialPorosityConst.h"
#include "PorousFlowMaterialMassFractionBuilder.h"
#include "PorousFlowMaterialPermeabilityConst.h"
#include "PorousFlowMaterialRelativePermeabilityConst.h"
#include "PorousFlowMaterialRelativePermeabilityCorey.h"
#include "PorousFlowMaterialRelativePermeabilityLinear.h"
#include "PorousFlowMaterialViscosityConst.h"
#include "PorousFlowMaterialJoiner.h"
#include "PorousFlowMaterialJoinerOld.h"

// Postprocessors
#include "PorousFlowFluidMass.h"

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
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
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
  registerKernel(PorousFlowTestKernel);

  // UserObjects
  registerUserObject(PorousFlowDictator);

  // Materials
  registerMaterial(PorousFlowMaterial1PhaseP_VG);
  registerMaterial(PorousFlowMaterial2PhasePS);
  registerMaterial(PorousFlowMaterial2PhasePP_VG);
  registerMaterial(PorousFlowMaterialDensityConstBulk);
  registerMaterial(PorousFlowMaterialPorosityConst);
  registerMaterial(PorousFlowMaterialMassFractionBuilder);
  registerMaterial(PorousFlowMaterialPermeabilityConst);
  registerMaterial(PorousFlowMaterialRelativePermeabilityConst);
  registerMaterial(PorousFlowMaterialRelativePermeabilityCorey);
  registerMaterial(PorousFlowMaterialRelativePermeabilityLinear);
  registerMaterial(PorousFlowMaterialViscosityConst);
  registerMaterial(PorousFlowMaterialJoiner);
  registerMaterial(PorousFlowMaterialJoinerOld);

  // Postprocessors
  registerPostprocessor(PorousFlowFluidMass);
}

// External entry point for dynamic syntax association
extern "C" void PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PorousFlowApp::associateSyntax(syntax, action_factory); }
void
PorousFlowApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
