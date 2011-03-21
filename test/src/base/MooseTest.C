#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"

#include "CoupledConvection.h"
#include "ForcingFn.h"
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

#include "PolyConstantAux.h"
#include "MMSConstantAux.h"

#include "MTBC.h"
#include "PolyCoupledDirichletBC.h"
#include "MMSCoupledDirichletBC.h"

#include "EmptyMaterial.h"
#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"

namespace MooseTest
{

void registerObjects()
{
  // Kernels
  registerObject(CoupledConvection);
  registerObject(ForcingFn);
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
  registerObject(PolyConstantAux);
  registerObject(MMSConstantAux);

  // Boundary Conditions
  registerObject(MTBC);
  registerObject(PolyCoupledDirichletBC);
  registerObject(MMSCoupledDirichletBC);

  // Materials
  registerObject(EmptyMaterial);
  registerObject(MTMaterial);
  registerObject(Diff1Material);
  registerObject(Diff2Material);
}

} // namespace
