#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"

#include "MooseTestApp.h"

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
#include "GenericDiffusion.h"
#include "ForcingFunctionXYZ0.h"
#include "TEJumpFFN.h"
#include "NanKernel.h"
#include "ExceptionKernel.h"
#include "MatConvection.h"
#include "PPSDiffusion.h"
#include "DotCouplingKernel.h"
#include "UserObjectKernel.h"
#include "DiffusionPrecompute.h"
#include "ConvectionPrecompute.h"
#include "CoupledKernelGradTest.h"
#include "CoupledKernelValueTest.h"

#include "CoupledAux.h"
#include "CoupledGradAux.h"
#include "PolyConstantAux.h"
#include "MMSConstantAux.h"
#include "MultipleUpdateAux.h"
#include "PeriodicDistanceAux.h"
#include "MatPropUserObjectAux.h"
#include "NodalNormalComponentAux.h"
#include "SumNodalValuesAux.h"

#include "MTBC.h"
#include "PolyCoupledDirichletBC.h"
#include "MMSCoupledDirichletBC.h"
#include "DirichletBCfuncXYZ0.h"
#include "DirichletBCfuncXYZ1.h"
#include "TEJumpBC.h"
#include "OnOffDirichletBC.h"
#include "OnOffNeumannBC.h"
#include "DivergenceBC.h"
#include "ScalarVarBC.h"

#include "TEIC.h"

#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"
#include "StatefulMaterial.h"
#include "SpatialStatefulMaterial.h"
#include "ComputingInitialTest.h"
#include "StatefulTest.h"
#include "StatefulSpatialTest.h"
#include "CoupledMaterial.h"
#include "CoupledMaterial2.h"
#include "LinearInterpolationMaterial.h"
#include "VarCouplingMaterial.h"

#include "DGMatDiffusion.h"
#include "DGMDDBC.h"
#include "DGFunctionConvectionDirichletBC.h"
#include "CoupledKernelGradBC.h"
#include "PenaltyDirichletBC.h"
#include "FunctionPenaltyDirichletBC.h"

#include "ExplicitODE.h"
#include "ImplicitODEx.h"
#include "ImplicitODEy.h"
#include "AlphaCED.h"

#include "EqualValueNodalConstraint.h"
// user objects
#include "MTUserObject.h"
#include "RandomHitUserObject.h"
#include "RandomHitSolutionModifier.h"
#include "MaterialPropertyUserObject.h"

#include "TimestepSetupFunction.h"
#include "PostprocessorFunction.h"
#include "MTPiecewiseConst1D.h"
#include "MTPiecewiseConst2D.h"
#include "MTPiecewiseConst3D.h"

// markers
#include "RandomHitMarker.h"

// meshes
#include "StripeMesh.h"

#include "ExceptionSteady.h"
#include "SteadyTransientExecutioner.h"
#include "AdaptAndModify.h"

// problems
#include "MooseTestProblem.h"

#include "ConvDiffMetaAction.h"
#include "AddLotsOfAuxVariablesAction.h"

// From MOOSE
#include "AddVariableAction.h"

void
MooseTest::registerApps()
{
  registerApp(MooseTestApp);
}

void
MooseTest::registerObjects(Factory & factory)
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
  registerKernel(GenericDiffusion);
  registerKernel(Advection0);
  registerKernel(AdvDiffReaction1);
  registerKernel(ForcingFunctionXYZ0);
  registerKernel(TEJumpFFN);
  registerKernel(NanKernel);
  registerKernel(ExceptionKernel);
  registerKernel(MatConvection);
  registerKernel(PPSDiffusion);
  registerKernel(DotCouplingKernel);
  registerKernel(UserObjectKernel);
  registerKernel(DiffusionPrecompute);
  registerKernel(ConvectionPrecompute);
  registerKernel(CoupledKernelGradTest);
  registerKernel(CoupledKernelValueTest);

  // Aux kernels
  registerAux(CoupledAux);
  registerAux(CoupledGradAux);
  registerAux(PolyConstantAux);
  registerAux(MMSConstantAux);
  registerAux(MultipleUpdateAux);
  registerAux(PeriodicDistanceAux);
  registerAux(MatPropUserObjectAux);
  registerAux(NodalNormalComponentAux);
  registerAux(SumNodalValuesAux);

  // DG kernels
  registerDGKernel(DGMatDiffusion);

  // Boundary Conditions
  registerBoundaryCondition(MTBC);
  registerBoundaryCondition(PolyCoupledDirichletBC);
  registerBoundaryCondition(MMSCoupledDirichletBC);
  registerBoundaryCondition(DirichletBCfuncXYZ0);
  registerBoundaryCondition(DirichletBCfuncXYZ1);
  registerBoundaryCondition(TEJumpBC);
  registerBoundaryCondition(OnOffDirichletBC);
  registerBoundaryCondition(OnOffNeumannBC);
  registerBoundaryCondition(ScalarVarBC);

  registerBoundaryCondition(DGMDDBC);
  registerBoundaryCondition(DGFunctionConvectionDirichletBC);
  registerBoundaryCondition(PenaltyDirichletBC);
  registerBoundaryCondition(FunctionPenaltyDirichletBC);

  registerBoundaryCondition(CoupledKernelGradBC);

  registerBoundaryCondition(DivergenceBC);

  // Initial conditions
  registerInitialCondition(TEIC);

  // Materials
  registerMaterial(MTMaterial);
  registerMaterial(Diff1Material);
  registerMaterial(Diff2Material);
  registerMaterial(StatefulMaterial);
  registerMaterial(SpatialStatefulMaterial);
  registerMaterial(ComputingInitialTest);
  registerMaterial(StatefulTest);
  registerMaterial(StatefulSpatialTest);
  registerMaterial(CoupledMaterial);
  registerMaterial(CoupledMaterial2);
  registerMaterial(LinearInterpolationMaterial);
  registerMaterial(VarCouplingMaterial);

  registerScalarKernel(ExplicitODE);
  registerScalarKernel(ImplicitODEx);
  registerScalarKernel(ImplicitODEy);
  registerScalarKernel(AlphaCED);

  // Functions
  registerFunction(TimestepSetupFunction);
  registerFunction(PostprocessorFunction);
  registerFunction(MTPiecewiseConst1D);
  registerFunction(MTPiecewiseConst2D);
  registerFunction(MTPiecewiseConst3D);

  // meshes
  registerObject(StripeMesh);

  registerConstraint(EqualValueNodalConstraint);

  registerUserObject(MTUserObject);
  registerUserObject(RandomHitUserObject);
  registerUserObject(RandomHitSolutionModifier);
  registerUserObject(MaterialPropertyUserObject);

  registerMarker(RandomHitMarker);

  registerExecutioner(ExceptionSteady);
  registerExecutioner(SteadyTransientExecutioner);
  registerExecutioner(AdaptAndModify);

  registerProblem(MooseTestProblem);
}

void
MooseTest::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  // and add more
  registerAction(ConvDiffMetaAction, "meta_action");
  registerAction(AddLotsOfAuxVariablesAction, "meta_action");
  syntax.registerActionSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
  syntax.registerActionSyntax("AddVariableAction", "MoreAuxVariables/*", "add_aux_variable");
  syntax.registerActionSyntax("AddLotsOfAuxVariablesAction", "LotsOfAuxVariables/*", "add_variable");
}
