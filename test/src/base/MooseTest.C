#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"

#include "Diffusion.h"
#include "CoupledForce.h"
#include "CoupledConvection.h"
#include "ForcingFn.h"
#include "UserForcingFunction.h"
#include "BodyForce.h"
#include "ImplicitEuler.h"
#include "MatDiffusion.h"
#include "DiffMKernel.h"
#include "GaussContForcing.h"
#include "CoefDiffusion.h"
#include "Convection.h"
#include "PolyDiffusion.h"
#include "PolyConvection.h"
#include "PolyForcing.h"
#include "PolyReaction.h"
#include "MMSImplicitEuler.h"
#include "MMSDiffusion.h"
#include "MMSConvection.h"
#include "MMSForcing.h"
#include "MMSReaction.h"

#include "CoupledAux.h"
#include "ConstantAux.h"
#include "PolyConstantAux.h"
#include "MMSConstantAux.h"
#include "FunctionAux.h"

#include "DirichletBC.h"
#include "NeumannBC.h"
#include "FunctionDirichletBC.h"
#include "FunctionNeumannBC.h"
#include "MTBC.h"
#include "MatchedValueBC.h"
#include "PolyCoupledDirichletBC.h"
#include "MMSCoupledDirichletBC.h"

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
  registerObject(CoupledForce);
  registerObject(CoupledConvection);
  registerObject(ForcingFn);
  registerObject(UserForcingFunction);
  registerObject(BodyForce);
  registerObject(ImplicitEuler);
  registerObject(MatDiffusion);
  registerObject(DiffMKernel);
  registerObject(GaussContForcing);
  registerObject(CoefDiffusion);
  registerObject(Convection);
  registerObject(PolyDiffusion);
  registerObject(PolyConvection);
  registerObject(PolyForcing);
  registerObject(PolyReaction);
  registerObject(MMSImplicitEuler);
  registerObject(MMSDiffusion);
  registerObject(MMSConvection);
  registerObject(MMSForcing);
  registerObject(MMSReaction);

  // Aux kernels
  registerObject(CoupledAux);
  registerObject(ConstantAux);
  registerObject(PolyConstantAux);
  registerObject(MMSConstantAux);
  registerObject(FunctionAux);

  // Boundary Conditions
  registerObject(DirichletBC);
  registerObject(NeumannBC);
  registerObject(FunctionDirichletBC);
  registerObject(FunctionNeumannBC);
  registerObject(MTBC);
  registerObject(MatchedValueBC);
  registerObject(PolyCoupledDirichletBC);
  registerObject(MMSCoupledDirichletBC);

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
