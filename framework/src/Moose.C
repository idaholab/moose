#include "KernelFactory.h"
#include "BodyForce.h"
#include "Diffusion.h"
#include "Reaction.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"
#include "VacuumBC.h"
#include "MatchedValueBC.h"

#include "ImplicitEuler.h"
#include "ImplicitBackwardDifference2.h"

#include "MaterialFactory.h"
#include "Constant.h"
#include "UO2.h"
#include "PorousMedia.h"
#include "Moose.h"

void
Moose::registerObjects()
{
  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  KernelFactory::instance()->registerKernel<Diffusion>("Diffusion");
  KernelFactory::instance()->registerKernel<Reaction>("Reaction");
  KernelFactory::instance()->registerKernel<ImplicitEuler>("ImplicitEuler");
  KernelFactory::instance()->registerKernel<ImplicitBackwardDifference2>("ImplicitBackwardDifference2");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
  BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");
  BCFactory::instance()->registerBC<VacuumBC>("VacuumBC");
  BCFactory::instance()->registerBC<MatchedValueBC>("MatchedValueBC");

  MaterialFactory::instance()->registerMaterial<Constant>("Constant");
  MaterialFactory::instance()->registerMaterial<UO2>("UO2");
  MaterialFactory::instance()->registerMaterial<PorousMedia>("PorousMedia");
}
