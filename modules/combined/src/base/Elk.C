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
#include "MiscModule.h"

void
Elk::registerObjects()
{
  SolidMechanics::registerObjects();
  TensorMechanics::registerObjects();
  PhaseField::registerObjects();
  Contact::registerObjects();
  HeatConduction::registerObjects();
  NavierStokes::registerObjects();
  LinearElasticity::registerObjects();
  FluidMassEnergyBalance::registerObjects();
  Misc::registerObjects();
}

void
Elk::associateSyntax(Syntax & syntax)
{
  SolidMechanics::associateSyntax(syntax);
  TensorMechanics::associateSyntax(syntax);
  Contact::associateSyntax(syntax);
  HeatConduction::associateSyntax(syntax);
}

