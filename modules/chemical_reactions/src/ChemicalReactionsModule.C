#include "ChemicalReactionsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

#include "PrimaryTimeDerivative.h"
#include "PrimaryConvection.h"
#include "PrimaryDiffusion.h"
#include "CoupledBEEquilibriumSub.h"
#include "CoupledConvectionReactionSub.h"
#include "CoupledDiffusionReactionSub.h"
#include "CoupledBEKinetic.h"

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

void
Elk::ChemicalReactions::registerObjects(Factory & factory)
{
  registerKernel(PrimaryTimeDerivative);
  registerKernel(PrimaryConvection);
  registerKernel(PrimaryDiffusion);
  registerKernel(CoupledBEEquilibriumSub);
  registerKernel(CoupledConvectionReactionSub);
  registerKernel(CoupledDiffusionReactionSub);
  registerKernel(CoupledBEKinetic);

  registerAux(AqueousEquilibriumRxnAux);
  registerAux(KineticDisPreConcAux);
  registerAux(KineticDisPreRateAux);
    
  registerBoundaryCondition(ChemicalOutFlowBC);
}

void
Elk::ChemicalReactions::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
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
}
