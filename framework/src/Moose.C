#include "KernelFactory.h"
#include "BodyForce.h"
#include "Diffusion.h"
#include "Reaction.h"
#include "CoupledForce.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "SinDirichletBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"
#include "VacuumBC.h"
#include "MatchedValueBC.h"
#include "ConvectiveFluxBC.h"
#include "WeakGradientBC.h"

#include "AuxFactory.h"
#include "ConstantAux.h"
#include "CoupledAux.h"

#include "ImplicitEuler.h"
#include "ImplicitBackwardDifference2.h"

#include "MaterialFactory.h"
#include "EmptyMaterial.h"

#include "Moose.h"

#include "ParallelUniqueId.h"

void
Moose::registerObjects()
{
  static bool first = true;
  if(first)
  {
    first = false;
    ParallelUniqueId::initialize();

    Kernel::sizeEverything();
    BoundaryCondition::sizeEverything();
    AuxKernel::sizeEverything();
  }

  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  KernelFactory::instance()->registerKernel<Diffusion>("Diffusion");
  KernelFactory::instance()->registerKernel<Reaction>("Reaction");
  KernelFactory::instance()->registerKernel<ImplicitEuler>("ImplicitEuler");
  KernelFactory::instance()->registerKernel<ImplicitBackwardDifference2>("ImplicitBackwardDifference2");
  KernelFactory::instance()->registerKernel<CoupledForce>("CoupledForce");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<SinDirichletBC>("SinDirichletBC");
  BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
  BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");
  BCFactory::instance()->registerBC<VacuumBC>("VacuumBC");
  BCFactory::instance()->registerBC<MatchedValueBC>("MatchedValueBC");
  BCFactory::instance()->registerBC<ConvectiveFluxBC>("ConvectiveFluxBC");
  BCFactory::instance()->registerBC<WeakGradientBC>("WeakGradientBC");

  AuxFactory::instance()->registerAux<ConstantAux>("ConstantAux");
  AuxFactory::instance()->registerAux<CoupledAux>("CoupledAux");

  MaterialFactory::instance()->registerMaterial<EmptyMaterial>("EmptyMaterial");
}

/******************
 * Global Variables
 * ****************/
THREAD_ID Moose::current_thread_id;

Mesh * Moose::mesh;
EquationSystems * Moose::equation_system;

enum Moose::GeomType;
Moose::GeomType Moose::geom_type = Moose::XYZ;

bool Moose::no_fe_reinit = false;
