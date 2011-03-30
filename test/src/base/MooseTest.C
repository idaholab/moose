#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"
#include "ActionFactory.h"

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
#include "AdvDiffReaction1.h"
#include "Advection0.h"
#include "CN2AdvDiffReaction1.h"
#include "Diffusion0.h"
#include "ForcingFunctionXYZ0.h"
#include "TEJumpFFN.h"

#include "PolyConstantAux.h"
#include "MMSConstantAux.h"

#include "MTBC.h"
#include "PolyCoupledDirichletBC.h"
#include "MMSCoupledDirichletBC.h"
#include "DirichletBCfuncXYZ0.h"
#include "DirichletBCfuncXYZ1.h"
#include "TEJumpBC.h"

#include "TEIC.h"

#include "EmptyMaterial.h"
#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"
#include "StatefulMaterial.h"
#include "StatefulTest.h"

#include "ConvDiffMetaAction.h"

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
  registerObject(Diffusion0);
  registerObject(Advection0);
  registerObject(AdvDiffReaction1);
  registerObject(CN2AdvDiffReaction1);
  registerObject(ForcingFunctionXYZ0);
  registerObject(TEJumpFFN);

  // Aux kernels
  registerObject(PolyConstantAux);
  registerObject(MMSConstantAux);

  // Boundary Conditions
  registerObject(MTBC);
  registerObject(PolyCoupledDirichletBC);
  registerObject(MMSCoupledDirichletBC);
  registerObject(DirichletBCfuncXYZ0);
  registerObject(DirichletBCfuncXYZ1);
  registerObject(TEJumpBC);

  // Initial conditions
  registerObject(TEIC);

  // Materials
  registerObject(EmptyMaterial);
  registerObject(MTMaterial);
  registerObject(Diff1Material);
  registerObject(Diff2Material);
  registerObject(StatefulMaterial);
  registerObject(StatefulTest);

  registerAction(ConvDiffMetaAction, "ConvectionDiffusion", "meta_action");
}

} // namespace
