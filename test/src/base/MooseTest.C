#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"

#include "Diffusion.h"
#include "Coupled.h"
#include "CoupledForce.h"
#include "ForcingFn.h"
#include "UserForcingFunction.h"
#include "BodyForce.h"
#include "ImplicitEuler.h"
#include "MatDiffusion.h"
#include "DiffMKernel.h"

#include "CoupledAux.h"
#include "ConstantAux.h"
#include "FunctionAux.h"

#include "DirichletBC.h"
#include "NeumannBC.h"
#include "FunctionDirichletBC.h"
#include "MTBC.h"

#include "ConstantIC.h"
#include "BoundingBoxIC.h"

#include "EmptyMaterial.h"
#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"

namespace MooseTest
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
  registerObject(MatDiffusion);
  registerObject(DiffMKernel);

  // Aux kernels
  registerObject(CoupledAux);
  registerObject(ConstantAux);
  registerObject(FunctionAux);

  // Boundary Conditions
  registerObject(DirichletBC);
  registerObject(NeumannBC);
  registerObject(FunctionDirichletBC);
  registerObject(MTBC);

  // Initial Conditions 
  registerObject(ConstantIC);
  registerObject(BoundingBoxIC);

  // Materials
  registerObject(EmptyMaterial);
  registerObject(MTMaterial);
  registerObject(Diff1Material);
  registerObject(Diff2Material);
}

} // namespace
