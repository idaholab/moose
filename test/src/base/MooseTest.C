#include "MooseTest.h"
#include "Moose.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "ProblemFactory.h"

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
#include "DivergenceBC.h"

#include "TEIC.h"

#include "MTMaterial.h"
#include "Diff1Material.h"
#include "Diff2Material.h"
#include "StatefulMaterial.h"
#include "ComputingInitialTest.h"
#include "StatefulTest.h"
#include "StatefulSpatialTest.h"
#include "CoupledMaterial.h"
#include "CoupledMaterial2.h"
#include "LinearInterpolationMaterial.h"
#include "VarCouplingMaterial.h"

#include "DGMatDiffusion.h"
#include "EnhancedDGMatDiffusion.h"
#include "DGMDDBC.h"
#include "DGFunctionConvectionDirichletBC.h"
#include "CoupledKernelGradBC.h"
#include "PenaltyDirichletBC.h"
#include "FunctionPenaltyDirichletBC.h"

#include "ExplicitODE.h"
#include "ImplicitODEx.h"
#include "ImplicitODEy.h"

#include "EqualValueNodalConstraint.h"
#include "MTUserObject.h"

#include "TimestepSetupFunction.h"
#include "PostprocessorFunction.h"
#include "MTPiecewiseConst1D.h"
#include "MTPiecewiseConst2D.h"
#include "MTPiecewiseConst3D.h"

// meshes
#include "StripeMesh.h"

// postprocessors
#include "UserObjectPPS.h"

#include "ExceptionSteady.h"
#include "SteadyTransientExecutioner.h"

// problems
#include "MooseTestProblem.h"

#include "ConvDiffMetaAction.h"
#include "AddLotsOfAuxVariablesAction.h"

// From MOOSE
#include "AddVariableAction.h"



MooseTestApp::MooseTestApp(int argc, char *argv[]) :
    MooseApp(argc, argv)
{
  registerObjects();
  associateSyntax();
}

void
MooseTestApp::registerObjects()
{
  Moose::registerObjects();

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

  // Functions
  registerFunction(TimestepSetupFunction);
  registerFunction(PostprocessorFunction);
  registerFunction(MTPiecewiseConst1D);
  registerFunction(MTPiecewiseConst2D);
  registerFunction(MTPiecewiseConst3D);

  // meshes
  registerObject(StripeMesh);

  registerConstraint(EqualValueNodalConstraint);

  registerPostprocessor(UserObjectPPS);

  registerUserObject(MTUserObject);

  registerExecutioner(ExceptionSteady);
  registerExecutioner(SteadyTransientExecutioner);

  registerProblem(MooseTestProblem);
}

void
MooseTestApp::associateSyntax()
{
  // add MOOSE basic syntax
  Moose::associateSyntax(_syntax);

  // and add more
  registerAction(ConvDiffMetaAction, "meta_action");
  registerAction(AddLotsOfAuxVariablesAction, "meta_action");
  _syntax.registerActionSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
  _syntax.registerActionSyntax("AddVariableAction", "MoreAuxVariables/*", "add_variable");
  _syntax.registerActionSyntax("AddLotsOfAuxVariablesAction", "LotsOfAuxVariables/*", "add_variable");
}
