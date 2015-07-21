/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "PrimaryTimeDerivative.h"
#include "PrimaryConvection.h"
#include "PrimaryDiffusion.h"
#include "CoupledBEEquilibriumSub.h"
#include "CoupledConvectionReactionSub.h"
#include "CoupledDiffusionReactionSub.h"
#include "CoupledBEKinetic.h"
#include "DesorptionFromMatrix.h"
#include "DesorptionToPorespace.h"

#include "AqueousEquilibriumRxnAux.h"
#include "KineticDisPreConcAux.h"
#include "KineticDisPreRateAux.h"

#include "AddPrimarySpeciesAction.h"
#include "AddSecondarySpeciesAction.h"
#include "AddCoupledEqSpeciesKernelsAction.h"
#include "AddCoupledEqSpeciesAuxKernelsAction.h"
#include "AddCoupledSolidKinSpeciesKernelsAction.h"
#include "AddCoupledSolidKinSpeciesAuxKernelsAction.h"

#include "ChemicalOutFlowBC.h"

#include "LangmuirMaterial.h"
#include "MollifiedLangmuirMaterial.h"

template<>
InputParameters validParams<ChemicalReactionsApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;

  return params;
}

ChemicalReactionsApp::ChemicalReactionsApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  ChemicalReactionsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ChemicalReactionsApp::associateSyntax(_syntax, _action_factory);
}

ChemicalReactionsApp::~ChemicalReactionsApp()
{
}

// External entry point for dynamic application loading
extern "C" void ChemicalReactionsApp__registerApps() { ChemicalReactionsApp::registerApps(); }
void
ChemicalReactionsApp::registerApps()
{
#undef  registerApp
#define registerApp(name) AppFactory::instance().reg<name>(#name)
  registerApp(ChemicalReactionsApp);
#undef  registerApp
#define registerApp(name) AppFactory::instance().regLegacy<name>(#name)
}

// External entry point for dynamic object registration
extern "C" void ChemicalReactionsApp__registerObjects(Factory & factory) { ChemicalReactionsApp::registerObjects(factory); }
void
ChemicalReactionsApp::registerObjects(Factory & factory)
{
#undef registerObject
#define registerObject(name) factory.reg<name>(stringifyName(name))

  registerKernel(PrimaryTimeDerivative);
  registerKernel(PrimaryConvection);
  registerKernel(PrimaryDiffusion);
  registerKernel(CoupledBEEquilibriumSub);
  registerKernel(CoupledConvectionReactionSub);
  registerKernel(CoupledDiffusionReactionSub);
  registerKernel(CoupledBEKinetic);
  registerKernel(DesorptionFromMatrix);
  registerKernel(DesorptionToPorespace);

  registerAux(AqueousEquilibriumRxnAux);
  registerAux(KineticDisPreConcAux);
  registerAux(KineticDisPreRateAux);

  registerBoundaryCondition(ChemicalOutFlowBC);

  registerMaterial(LangmuirMaterial);
  registerMaterial(MollifiedLangmuirMaterial);

#undef registerObject
#define registerObject(name) factory.regLegacy<name>(stringifyName(name))

}

// External entry point for dynamic syntax association
extern "C" void ChemicalReactionsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { ChemicalReactionsApp::associateSyntax(syntax, action_factory); }
void
ChemicalReactionsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{

#undef registerAction
#define registerAction(tplt, action) action_factory.reg<tplt>(stringifyName(tplt), action)


  syntax.registerActionSyntax("AddPrimarySpeciesAction", "ReactionNetwork");
  syntax.registerActionSyntax("AddSecondarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  syntax.registerActionSyntax("AddSecondarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  syntax.registerActionSyntax("AddCoupledEqSpeciesKernelsAction", "ReactionNetwork/AqueousEquilibriumReactions");
  syntax.registerActionSyntax("AddCoupledEqSpeciesAuxKernelsAction", "ReactionNetwork/AqueousEquilibriumReactions");
  syntax.registerActionSyntax("AddCoupledSolidKinSpeciesKernelsAction", "ReactionNetwork/SolidKineticReactions");
  syntax.registerActionSyntax("AddCoupledSolidKinSpeciesAuxKernelsAction", "ReactionNetwork/SolidKineticReactions");
  registerAction(AddPrimarySpeciesAction, "add_variable");
  registerAction(AddSecondarySpeciesAction, "add_aux_variable");
  registerAction(AddCoupledEqSpeciesKernelsAction, "add_kernel");
  registerAction(AddCoupledEqSpeciesAuxKernelsAction, "add_aux_kernel");
  registerAction(AddCoupledSolidKinSpeciesKernelsAction, "add_kernel");
  registerAction(AddCoupledSolidKinSpeciesAuxKernelsAction, "add_aux_kernel");

#undef registerAction
#define registerAction(tplt, action) action_factory.regLegacy<tplt>(stringifyName(tplt), action)
}


// DEPRECATED CONSTRUCTOR
ChemicalReactionsApp::ChemicalReactionsApp(const std::string & deprecated_name, InputParameters parameters) :
    MooseApp(deprecated_name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  ChemicalReactionsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ChemicalReactionsApp::associateSyntax(_syntax, _action_factory);
}
