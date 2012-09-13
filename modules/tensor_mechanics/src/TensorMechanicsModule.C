#include "TensorMechanicsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// tensor_mechanics
#include "TensorMechanicsAction.h"
#include "StressDivergenceTensors.h"
#include "StressDivergenceTensorsTonks.h"
#include "LinearElasticMaterial.h"
#include "TensorElasticMaterial.h"
#include "RankTwoAux.h"
#include "RankTwoTonksAux.h"
#include "RealTensorValueAux.h"
#include "RankFourAux.h"
#include "RankFourTonksAux.h"

void
Elk::TensorMechanics::registerObjects()
{
  // tensor_mechanics
  registerKernel(StressDivergenceTensors);
  registerKernel(StressDivergenceTensorsTonks);

  registerMaterial(LinearElasticMaterial);
  registerMaterial(TensorElasticMaterial);


  registerAux(RankTwoAux);
  registerAux(RankTwoTonksAux);
  registerAux(RealTensorValueAux);
  registerAux(RankFourAux);
  registerAux(RankFourTonksAux);
}

void
Elk::TensorMechanics::associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("TensorMechanicsAction", "TensorMechanics/*");

  registerAction(TensorMechanicsAction, "add_kernel");
}
