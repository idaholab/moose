#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

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
#include "NanKernel.h"
#include "MatConvection.h"
#include "PPSDiffusion.h"

#include "PolyConstantAux.h"
#include "MMSConstantAux.h"
#include "DoNothingAux.h"
#include "MultipleUpdateAux.h"

#include "MTBC.h"
#include "PolyCoupledDirichletBC.h"
#include "MMSCoupledDirichletBC.h"
#include "DirichletBCfuncXYZ0.h"
#include "DirichletBCfuncXYZ1.h"
#include "TEJumpBC.h"
#include "DGFunctionDiffusionDirichletBC.h"

#include "TEIC.h"

#include "EmptyMaterial.h"
#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"
#include "StatefulMaterial.h"
#include "StatefulTest.h"
#include "StatefulSpatialTest.h"
#include "CoupledMaterial.h"

#include "ConvDiffMetaAction.h"

#include "DGMatDiffusion.h"
#include "EnhancedDGMatDiffusion.h"
#include "DGMDDBC.h"
#include "DGFunctionConvectionDirichletBC.h"


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
  registerObject(NanKernel);
  registerKernel(MatConvection);
  registerKernel(PPSDiffusion);

  // Aux kernels
  registerObject(PolyConstantAux);
  registerObject(MMSConstantAux);
  registerObject(DoNothingAux);
  registerObject(MultipleUpdateAux);

  // DG kernels
  registerDGKernel(DGMatDiffusion);
  registerDGKernel(EnhancedDGMatDiffusion);

  // Boundary Conditions
  registerObject(MTBC);
  registerObject(PolyCoupledDirichletBC);
  registerObject(MMSCoupledDirichletBC);
  registerObject(DirichletBCfuncXYZ0);
  registerObject(DirichletBCfuncXYZ1);
  registerObject(TEJumpBC);

  registerBoundaryCondition(DGFunctionDiffusionDirichletBC);
  registerBoundaryCondition(DGMDDBC);
  registerBoundaryCondition(DGFunctionConvectionDirichletBC);

  // Initial conditions
  registerObject(TEIC);

  // Materials
  registerObject(EmptyMaterial);
  registerObject(MTMaterial);
  registerObject(Diff1Material);
  registerObject(Diff2Material);
  registerObject(StatefulMaterial);
  registerObject(StatefulTest);
  registerObject(StatefulSpatialTest);
  registerMaterial(CoupledMaterial);

  registerAction(ConvDiffMetaAction, "meta_action");
}

} // namespace
