//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "PrimaryTimeDerivative.h"
#include "PrimaryConvection.h"
#include "PrimaryDiffusion.h"
#include "CoupledBEEquilibriumSub.h"
#include "CoupledConvectionReactionSub.h"
#include "CoupledDiffusionReactionSub.h"
#include "CoupledBEKinetic.h"
#include "DesorptionFromMatrix.h"
#include "DesorptionToPorespace.h"
#include "DarcyFluxPressure.h"

#include "AqueousEquilibriumRxnAux.h"
#include "KineticDisPreConcAux.h"
#include "KineticDisPreRateAux.h"
#include "PHAux.h"

#include "AddPrimarySpeciesAction.h"
#include "AddSecondarySpeciesAction.h"
#include "AddCoupledEqSpeciesAction.h"
#include "AddCoupledSolidKinSpeciesAction.h"

#include "ChemicalOutFlowBC.h"

#include "LangmuirMaterial.h"
#include "MollifiedLangmuirMaterial.h"

#include "TotalMineralVolumeFraction.h"

template <>
InputParameters
validParams<ChemicalReactionsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ChemicalReactionsApp::ChemicalReactionsApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ChemicalReactionsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ChemicalReactionsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ChemicalReactionsApp::registerExecFlags(_factory);
}

ChemicalReactionsApp::~ChemicalReactionsApp() {}

// External entry point for dynamic application loading
extern "C" void
ChemicalReactionsApp__registerApps()
{
  ChemicalReactionsApp::registerApps();
}
void
ChemicalReactionsApp::registerApps()
{
  registerApp(ChemicalReactionsApp);
}

// External entry point for dynamic object registration
extern "C" void
ChemicalReactionsApp__registerObjects(Factory & factory)
{
  ChemicalReactionsApp::registerObjects(factory);
}
void
ChemicalReactionsApp::registerObjects(Factory & factory)
{
  registerKernel(PrimaryTimeDerivative);
  registerKernel(PrimaryConvection);
  registerKernel(PrimaryDiffusion);
  registerKernel(CoupledBEEquilibriumSub);
  registerKernel(CoupledConvectionReactionSub);
  registerKernel(CoupledDiffusionReactionSub);
  registerKernel(CoupledBEKinetic);
  registerKernel(DesorptionFromMatrix);
  registerKernel(DesorptionToPorespace);
  registerKernel(DarcyFluxPressure);

  registerAux(AqueousEquilibriumRxnAux);
  registerAux(KineticDisPreConcAux);
  registerAux(KineticDisPreRateAux);
  registerAux(PHAux);

  registerBoundaryCondition(ChemicalOutFlowBC);

  registerMaterial(LangmuirMaterial);
  registerMaterial(MollifiedLangmuirMaterial);

  registerPostprocessor(TotalMineralVolumeFraction);
}

// External entry point for dynamic syntax association
extern "C" void
ChemicalReactionsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
}
void
ChemicalReactionsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddCoupledEqSpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddCoupledSolidKinSpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerAction(AddPrimarySpeciesAction, "add_variable");
  registerAction(AddSecondarySpeciesAction, "add_aux_variable");
  registerAction(AddCoupledEqSpeciesAction, "add_kernel");
  registerAction(AddCoupledEqSpeciesAction, "add_aux_kernel");
  registerAction(AddCoupledSolidKinSpeciesAction, "add_kernel");
  registerAction(AddCoupledSolidKinSpeciesAction, "add_aux_kernel");
}

// External entry point for dynamic execute flag registration
extern "C" void
ChemicalReactionsApp__registerExecFlags(Factory & factory)
{
  ChemicalReactionsApp::registerExecFlags(factory);
}
void
ChemicalReactionsApp::registerExecFlags(Factory & /*factory*/)
{
}
