#include "TensorMechanicsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// tensor_mechanics
#include "TensorMechanicsAction.h"
#include "StressDivergenceTensors.h"
#include "LinearElasticMaterial.h"
#include "RankTwoAux.h"
#include "RealTensorValueAux.h"
#include "RankFourAux.h"

void
Elk::TensorMechanics::registerObjects()
{
  // tensor_mechanics
  registerKernel(StressDivergenceTensors);

  registerMaterial(LinearElasticMaterial);


  registerAux(RankTwoAux);
  registerAux(RealTensorValueAux);
  registerAux(RankFourAux);
}

void
Elk::TensorMechanics::associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("TensorMechanicsAction", "TensorMechanics/*");

  registerAction(TensorMechanicsAction, "add_kernel");
}
