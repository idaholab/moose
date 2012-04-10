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
#include "Diffusion0.h"
#include "ForcingFunctionXYZ0.h"
#include "TEJumpFFN.h"
#include "NanKernel.h"
#include "MatConvection.h"
#include "PPSDiffusion.h"
#include "DotCouplingKernel.h"

#include "CoupledAux.h"
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
#include "OnOffDirichletBC.h"
#include "OnOffNeumannBC.h"

#include "TEIC.h"

#include "EmptyMaterial.h"
#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"
#include "StatefulMaterial.h"
#include "StatefulTest.h"
#include "StatefulSpatialTest.h"
#include "CoupledMaterial.h"
#include "LinearInterpolationMaterial.h"

#include "ConvDiffMetaAction.h"

#include "DGMatDiffusion.h"
#include "EnhancedDGMatDiffusion.h"
#include "DGMDDBC.h"
#include "DGFunctionConvectionDirichletBC.h"

#include "ExplicitODE.h"
#include "ImplicitODEx.h"
#include "ImplicitODEy.h"

#include "EqualValueNodalConstraint.h"

// meshes
#include "StripeMesh.h"


namespace MooseTest
{

void registerObjects()
{
  // Kernels
  registerKernel(CoupledConvection);
  registerKernel(ForcingFn);
  registerKernel(MatDiffusion);
  registerKernel(DiffMKernel);
  registerKernel(GaussContForcing);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerKernel(PolyDiffusion);
  registerKernel(PolyConvection);
  registerKernel(PolyForcing);
  registerKernel(PolyReaction);
  registerKernel(MMSImplicitEuler);
  registerKernel(MMSDiffusion);
  registerKernel(MMSConvection);
  registerKernel(MMSForcing);
  registerKernel(MMSReaction);
  registerKernel(Diffusion0);
  registerKernel(Advection0);
  registerKernel(AdvDiffReaction1);
  registerKernel(ForcingFunctionXYZ0);
  registerKernel(TEJumpFFN);
  registerKernel(NanKernel);
  registerKernel(MatConvection);
  registerKernel(PPSDiffusion);
  registerKernel(DotCouplingKernel);

  // Aux kernels
  registerAux(CoupledAux);
  registerAux(PolyConstantAux);
  registerAux(MMSConstantAux);
  registerAux(DoNothingAux);
  registerAux(MultipleUpdateAux);

  // DG kernels
  registerDGKernel(DGMatDiffusion);
  registerDGKernel(EnhancedDGMatDiffusion);

  // Boundary Conditions
  registerBoundaryCondition(MTBC);
  registerBoundaryCondition(PolyCoupledDirichletBC);
  registerBoundaryCondition(MMSCoupledDirichletBC);
  registerBoundaryCondition(DirichletBCfuncXYZ0);
  registerBoundaryCondition(DirichletBCfuncXYZ1);
  registerBoundaryCondition(TEJumpBC);
  registerBoundaryCondition(OnOffDirichletBC);
  registerBoundaryCondition(OnOffNeumannBC);

  registerBoundaryCondition(DGMDDBC);
  registerBoundaryCondition(DGFunctionConvectionDirichletBC);

  // Initial conditions
  registerInitialCondition(TEIC);

  // Materials
  registerMaterial(EmptyMaterial);
  registerMaterial(MTMaterial);
  registerMaterial(Diff1Material);
  registerMaterial(Diff2Material);
  registerMaterial(StatefulMaterial);
  registerMaterial(StatefulTest);
  registerMaterial(StatefulSpatialTest);
  registerMaterial(CoupledMaterial);
  registerMaterial(LinearInterpolationMaterial);

  registerScalarKernel(ExplicitODE);
  registerScalarKernel(ImplicitODEx);
  registerScalarKernel(ImplicitODEy);

  registerAction(ConvDiffMetaAction, "meta_action");

  // meshes
  registerObject(StripeMesh);

  registerConstraint(EqualValueNodalConstraint);
}

} // namespace
