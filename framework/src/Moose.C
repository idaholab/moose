#include "KernelFactory.h"
#include "BodyForce.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"
#include "ImplicitEuler.h"

#include "MaterialFactory.h"
#include "Constant.h"
#include "UO2.h"

#include "Moose.h"

void
Moose::registerObjects()
{
  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  KernelFactory::instance()->registerKernel<ImplicitEuler>("ImplicitEuler");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
  BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");

  MaterialFactory::instance()->registerMaterial<Constant>("Constant");
  MaterialFactory::instance()->registerMaterial<UO2>("UO2");
}
