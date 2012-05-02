#include "Elk.h"
#include "Factory.h"
#include "ActionFactory.h"

#include "SolidMechanicsModule.h"
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
  PhaseField::registerObjects();
  Contact::registerObjects();
  HeatConduction::registerObjects();
  NavierStokes::registerObjects();
  LinearElasticity::registerObjects();
  FluidMassEnergyBalance::registerObjects();
  Misc::registerObjects();
}

void
Elk::associateSyntax()
{
  SolidMechanics::associateSyntax();
  Contact::associateSyntax();
  HeatConduction::associateSyntax();
}

