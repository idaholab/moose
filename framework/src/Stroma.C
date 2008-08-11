#include "KernelFactory.h"
#include "BodyForce.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"

#include "MaterialFactory.h"
#include "Constant.h"
#include "UO2.h"

#include "Stroma.h"

void
Stroma::registerObjects()
{
  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
  BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");

  MaterialFactory::instance()->registerMaterial<Constant>("Constant");
  MaterialFactory::instance()->registerMaterial<UO2>("UO2");
}
