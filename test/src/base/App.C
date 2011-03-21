#include "App.h"
#include "Moose.h"
#include "Factory.h"

#include "Diffusion.h"
#include "Coupled.h"
#include "CoupledForce.h"
#include "ForcingFn.h"
#include "UserForcingFunction.h"
#include "BodyForce.h"
#include "ImplicitEuler.h"

#include "CoupledAux.h"
#include "ConstantAux.h"

#include "DirichletBC.h"
#include "NeumannBC.h"
#include "FunctionDirichletBC.h"

#include "ConstantIC.h"
#include "BoundingBoxIC.h"

#include "EmptyMaterial.h"

namespace App
{

void registerObjects()
{
  // Kernels
  registerObject(Diffusion);
  registerObject(Coupled);
  registerObject(CoupledForce);
  registerObject(ForcingFn);
  registerObject(UserForcingFunction);
  registerObject(BodyForce);
  registerObject(ImplicitEuler);

  // Aux kernels
  registerObject(CoupledAux);
  registerObject(ConstantAux);

  // Boundary Conditions
  registerObject(DirichletBC);
  registerObject(NeumannBC);
  registerObject(FunctionDirichletBC);

  // Initial Conditions 
  registerObject(ConstantIC);
  registerObject(BoundingBoxIC);

  // Materials
  registerObject(EmptyMaterial);
}

}
