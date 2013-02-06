#include "Elk.h"
#include "Factory.h"
#include "ActionFactory.h"

#include "SolidMechanicsModule.h"
#include "TensorMechanicsModule.h"
#include "PhaseFieldModule.h"
#include "ContactModule.h"
#include "HeatConductionModule.h"
#include "NavierStokesModule.h"
#include "LinearElasticityModule.h"
#include "FluidMassEnergyBalanceModule.h"
#include "ChemicalReactionsModule.h"
#include "MiscModule.h"

void
Elk::registerObjects(Factory & factory)
{
  SolidMechanics::registerObjects(factory);
  TensorMechanics::registerObjects(factory);
  PhaseField::registerObjects(factory);
  Contact::registerObjects(factory);
  HeatConduction::registerObjects(factory);
  NavierStokes::registerObjects(factory);
  LinearElasticity::registerObjects(factory);
  FluidMassEnergyBalance::registerObjects(factory);
  ChemicalReactions::registerObjects(factory);
  Misc::registerObjects(factory);
}

void
Elk::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanics::associateSyntax(syntax, action_factory);
  TensorMechanics::associateSyntax(syntax, action_factory);
  Contact::associateSyntax(syntax, action_factory);
  HeatConduction::associateSyntax(syntax, action_factory);
  ChemicalReactions::associateSyntax(syntax, action_factory);
  Misc::associateSyntax(syntax, action_factory);
}
