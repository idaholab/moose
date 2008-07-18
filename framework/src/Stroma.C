#include "KernelFactory.h"

#include "BodyForce.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"

#include "Stroma.h"

void
Stroma::registerObjects()
{
  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
  BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");
}
