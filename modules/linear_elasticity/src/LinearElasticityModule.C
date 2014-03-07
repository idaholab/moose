#include "LinearElasticityModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// linear_elasticity
#include "LinearElasticityMaterial.h"
#include "SolidMechX.h"
#include "SolidMechY.h"
#include "SolidMechZ.h"
#include "SolidMechTempCoupleX.h"
#include "SolidMechTempCoupleY.h"
#include "SolidMechTempCoupleZ.h"

void
Elk::LinearElasticity::registerObjects(Factory & factory)
{
  // linear_elasticity
  registerMaterial(LinearElasticityMaterial);
  registerKernel(SolidMechX);
  registerKernel(SolidMechY);
  registerKernel(SolidMechZ);
  registerKernel(SolidMechTempCoupleX);
  registerKernel(SolidMechTempCoupleY);
  registerKernel(SolidMechTempCoupleZ);
}
