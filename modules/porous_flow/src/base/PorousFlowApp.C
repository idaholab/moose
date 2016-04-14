#include "PorousFlowApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// Kernels
#include "PorousFlowComponentMassTimeDerivative.h"
#include "PorousFlowAdvectiveFlux.h"

// UserObjects
#include "PorousFlowDictator.h"

// Materials
#include "PorousFlowMaterial2PhasePS.h"
#include "PorousFlowMaterialDensityConstBulk.h"
#include "PorousFlowMaterialDensityBuilder.h"
#include "PorousFlowMaterialPorosityConst.h"
#include "PorousFlowMaterialMassFractionBuilder.h"
#include "PorousFlowMaterialPermeabilityConst.h"
#include "PorousFlowMaterialRelativePermeabilityConst.h"
#include "PorousFlowMaterialViscosityConst.h"

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
  registerKernel(PorousFlowComponentMassTimeDerivative);
  registerKernel(PorousFlowAdvectiveFlux);

  // UserObjects
  registerUserObject(PorousFlowDictator);

  // Materials
  registerMaterial(PorousFlowMaterial2PhasePS);
  registerMaterial(PorousFlowMaterialDensityConstBulk);
  registerMaterial(PorousFlowMaterialDensityBuilder);
  registerMaterial(PorousFlowMaterialPorosityConst);
  registerMaterial(PorousFlowMaterialMassFractionBuilder);
  registerMaterial(PorousFlowMaterialPermeabilityConst);
  registerMaterial(PorousFlowMaterialRelativePermeabilityConst);
  registerMaterial(PorousFlowMaterialViscosityConst);
}

// External entry point for dynamic syntax association
extern "C" void PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PorousFlowApp::associateSyntax(syntax, action_factory); }
void
PorousFlowApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
