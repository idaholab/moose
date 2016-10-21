/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "MooseTestApp.h"
#include "Moose.h"
#include "Factory.h"
#include "MooseSyntax.h"

#include "ActionFactory.h"
#include "AppFactory.h"

#include "MooseTestApp.h"

#include "ConservativeAdvection.h"
#include "CoeffParamDiffusion.h"
#include "CoupledConvection.h"
#include "ForcingFn.h"
#include "MatDiffusion.h"
#include "DiffMKernel.h"
#include "GaussContForcing.h"
#include "CoefDiffusion.h"
#include "RestartDiffusion.h"
#include "MatCoefDiffusion.h"
#include "FuncCoefDiffusion.h"
#include "CoefReaction.h"
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
#include "NanAtCountKernel.h"
#include "ExceptionKernel.h"
#include "MatConvection.h"
#include "PPSDiffusion.h"
#include "DefaultPostprocessorDiffusion.h"
#include "DotCouplingKernel.h"
#include "UserObjectKernel.h"
#include "DiffusionPrecompute.h"
#include "ConvectionPrecompute.h"
#include "CoupledKernelGradTest.h"
#include "CoupledKernelValueTest.h"
#include "ScalarLagrangeMultiplier.h"
#include "SplineFFn.h"
#include "BlkResTestDiffusion.h"
#include "DiffTensorKernel.h"
#include "OptionallyCoupledForce.h"
#include "FDDiffusion.h"
#include "FDAdvection.h"
#include "MaterialEigenKernel.h"
#include "PHarmonic.h"
#include "PMassEigenKernel.h"
#include "CoupledEigenKernel.h"
#include "ConsoleMessageKernel.h"
#include "WrongJacobianDiffusion.h"
#include "DefaultMatPropConsumerKernel.h"
#include "DoNotCopyParametersKernel.h"
#include "CoupledAux.h"
#include "CoupledScalarAux.h"
#include "CoupledGradAux.h"
#include "PolyConstantAux.h"
#include "MMSConstantAux.h"
#include "MultipleUpdateAux.h"
#include "MultipleUpdateElemAux.h"
#include "PeriodicDistanceAux.h"
#include "MatPropUserObjectAux.h"
#include "SumNodalValuesAux.h"
#include "UniqueIDAux.h"
#include "RandomAux.h"
#include "PostprocessorAux.h"
#include "FluxAverageAux.h"
#include "OldMaterialAux.h"
#include "DotCouplingAux.h"
#include "VectorPostprocessorAux.h"
#include "ExampleShapeElementKernel.h"
#include "ExampleShapeElementKernel2.h"
#include "SimpleTestShapeElementKernel.h"

#include "RobinBC.h"
#include "InflowBC.h"
#include "OutflowBC.h"
#include "MTBC.h"
#include "PolyCoupledDirichletBC.h"
#include "MMSCoupledDirichletBC.h"
#include "DirichletBCfuncXYZ0.h"
#include "TEJumpBC.h"
#include "OnOffDirichletBC.h"
#include "OnOffNeumannBC.h"
#include "DivergenceBC.h"
#include "ScalarVarBC.h"
#include "BndTestDirichletBC.h"
#include "MatTestNeumannBC.h"
#include "MatDivergenceBC.h"
#include "CoupledDirichletBC.h"
#include "TestLapBC.h"

// ICs
#include "TEIC.h"
#include "MTICSum.h"
#include "MTICMult.h"
#include "DataStructIC.h"

// Materials
#include "MTMaterial.h"
#include "TypesMaterial.h"
#include "StatefulMaterial.h"
#include "SpatialStatefulMaterial.h"
#include "ComputingInitialTest.h"
#include "StatefulTest.h"
#include "StatefulSpatialTest.h"
#include "CoupledMaterial.h"
#include "CoupledMaterial2.h"
#include "LinearInterpolationMaterial.h"
#include "VarCouplingMaterial.h"
#include "VarCouplingMaterialEigen.h"
#include "BadStatefulMaterial.h"
#include "OutputTestMaterial.h"
#include "SumMaterial.h"
#include "VecRangeCheckMaterial.h"
#include "DerivativeMaterialInterfaceTestProvider.h"
#include "DerivativeMaterialInterfaceTestClient.h"
#include "DefaultMatPropConsumerMaterial.h"
#include "RandomMaterial.h"
#include "RecomputeMaterial.h"
#include "NewtonMaterial.h"
#include "ThrowMaterial.h"

#include "DGMatDiffusion.h"
#include "DGAdvection.h"
#include "DGMDDBC.h"
#include "DGFunctionConvectionDirichletBC.h"
#include "CoupledKernelGradBC.h"

#include "InterfaceDiffusion.h"

#include "ExplicitODE.h"
#include "ImplicitODEx.h"
#include "ImplicitODEy.h"
#include "AlphaCED.h"
#include "PostprocessorCED.h"
#include "VectorPostprocessorScalarKernel.h"

#include "EqualValueNodalConstraint.h"

// user objects
#include "MTUserObject.h"
#include "RandomHitUserObject.h"
#include "RandomHitSolutionModifier.h"
#include "MaterialPropertyUserObject.h"
#include "MaterialCopyUserObject.h"
#include "InsideUserObject.h"
#include "RestartableTypes.h"
#include "RestartableTypesChecker.h"
#include "PointerStoreError.h"
#include "PointerLoadError.h"
#include "VerifyElementUniqueID.h"
#include "VerifyNodalUniqueID.h"
#include "RandomElementalUserObject.h"
#include "TrackDiracFront.h"
#include "BoundaryUserObject.h"
#include "TestBoundaryRestrictableAssert.h"
#include "GetMaterialPropertyBoundaryBlockNamesTest.h"
#include "SetupInterfaceCount.h"
#include "ReadDoubleIndex.h"
#include "TestShapeElementUserObject.h"
#include "ExampleShapeElementUserObject.h"
#include "SimpleTestShapeElementUserObject.h"

// Postprocessors
#include "TestCopyInitialSolution.h"
#include "TestSerializedSolution.h"
#include "InsideValuePPS.h"
#include "BoundaryValuePPS.h"
#include "NumInternalSides.h"
#include "NumElemQPs.h"
#include "NumSideQPs.h"
#include "ElementL2Diff.h"
#include "TestPostprocessor.h"
#include "ElementSidePP.h"
#include "RealControlParameterReporter.h"
#include "ScalarCoupledPostprocessor.h"
#include "NumAdaptivityCycles.h"

// Functions
#include "TimestepSetupFunction.h"
#include "PostprocessorFunction.h"
#include "MTPiecewiseConst1D.h"
#include "MTPiecewiseConst2D.h"
#include "MTPiecewiseConst3D.h"
#include "TestSetupPostprocessorDataActionFunction.h"

// DiracKernels
#include "ReportingConstantSource.h"
#include "FrontSource.h"
#include "MaterialPointSource.h"
#include "MaterialMultiPointSource.h"
#include "CachingPointSource.h"
#include "BadCachingPointSource.h"
#include "NonlinearSource.h"

// markers
#include "RandomHitMarker.h"
#include "QPointMarker.h"
#include "CircleMarker.h"

// meshes
#include "StripeMesh.h"

#include "TestSteady.h"
#include "AdaptAndModify.h"

// problems
#include "MooseTestProblem.h"
#include "FailingProblem.h"

// actions
#include "ConvDiffMetaAction.h"
#include "AddLotsOfAuxVariablesAction.h"
#include "ApplyCoupledVariablesTestAction.h"
#include "AddLotsOfDiffusion.h"
#include "TestGetActionsAction.h"
#include "BadAddKernelAction.h"

// TimeSteppers
#include "TimeSequenceStepperFailTest.h"

// From MOOSE
#include "AddVariableAction.h"

// Outputs
#include "OutputObjectTest.h"

// Controls
#include "TestControl.h"

// Indicators
#include "MaterialTestIndicator.h"

template<>
InputParameters validParams<MooseTestApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}


MooseTestApp::MooseTestApp(const InputParameters & parameters):
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  MooseTestApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  MooseTestApp::associateSyntax(_syntax, _action_factory);
}

MooseTestApp::~MooseTestApp()
{
}

// External entry point for dynamic application loading
extern "C" void MooseTestApp__registerApps() { MooseTestApp::registerApps(); }
void
MooseTestApp::registerApps()
{
  registerApp(MooseTestApp);
}

// External entry point for dynamic object registration
extern "C" void MooseTestApp__registerObjects(Factory & factory) { MooseTestApp::registerObjects(factory); }
void
MooseTestApp::registerObjects(Factory & factory)
{
  // Kernels
  registerKernel(ConservativeAdvection);
  registerKernel(CoeffParamDiffusion);
  registerKernel(CoupledConvection);
  registerKernel(ForcingFn);
  registerKernel(MatDiffusion);
  registerKernel(DiffMKernel);
  registerKernel(GaussContForcing);
  registerKernel(CoefDiffusion);
  registerKernel(RestartDiffusion);
  registerKernel(MatCoefDiffusion);
  registerKernel(FuncCoefDiffusion);
  registerKernel(CoefReaction);
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
  registerKernel(NanAtCountKernel);
  registerKernel(ExceptionKernel);
  registerKernel(MatConvection);
  registerKernel(PPSDiffusion);
  registerKernel(DefaultPostprocessorDiffusion);
  registerKernel(DotCouplingKernel);
  registerKernel(UserObjectKernel);
  registerKernel(DiffusionPrecompute);
  registerKernel(ConvectionPrecompute);
  registerKernel(CoupledKernelGradTest);
  registerKernel(CoupledKernelValueTest);
  registerKernel(SplineFFn);
  registerKernel(BlkResTestDiffusion);
  registerKernel(DiffTensorKernel);
  registerKernel(ScalarLagrangeMultiplier);
  registerKernel(OptionallyCoupledForce);
  registerKernel(FDDiffusion);
  registerKernel(FDAdvection);
  registerKernel(MaterialEigenKernel);
  registerKernel(PHarmonic);
  registerKernel(PMassEigenKernel);
  registerKernel(CoupledEigenKernel);
  registerKernel(ConsoleMessageKernel);
  registerKernel(WrongJacobianDiffusion);
  registerKernel(DefaultMatPropConsumerKernel);
  registerKernel(DoNotCopyParametersKernel);
  registerKernel(ExampleShapeElementKernel);
  registerKernel(ExampleShapeElementKernel2);
  registerKernel(SimpleTestShapeElementKernel);

  // Aux kernels
  registerAux(CoupledAux);
  registerAux(CoupledScalarAux);
  registerAux(CoupledGradAux);
  registerAux(PolyConstantAux);
  registerAux(MMSConstantAux);
  registerAux(MultipleUpdateAux);
  registerAux(MultipleUpdateElemAux);
  registerAux(PeriodicDistanceAux);
  registerAux(MatPropUserObjectAux);
  registerAux(SumNodalValuesAux);
  registerAux(UniqueIDAux);
  registerAux(RandomAux);
  registerAux(PostprocessorAux);
  registerAux(FluxAverageAux);
  registerAux(OldMaterialAux);
  registerAux(DotCouplingAux);
  registerAux(VectorPostprocessorAux);

  // DG kernels
  registerDGKernel(DGMatDiffusion);
  registerDGKernel(DGAdvection);

  // Interface kernels
  registerInterfaceKernel(InterfaceDiffusion);

  // Boundary Conditions
  registerBoundaryCondition(RobinBC);
  registerBoundaryCondition(InflowBC);
  registerBoundaryCondition(OutflowBC);
  registerBoundaryCondition(MTBC);
  registerBoundaryCondition(PolyCoupledDirichletBC);
  registerBoundaryCondition(MMSCoupledDirichletBC);
  registerBoundaryCondition(DirichletBCfuncXYZ0);
  registerBoundaryCondition(TEJumpBC);
  registerBoundaryCondition(OnOffDirichletBC);
  registerBoundaryCondition(OnOffNeumannBC);
  registerBoundaryCondition(ScalarVarBC);
  registerBoundaryCondition(BndTestDirichletBC);
  registerBoundaryCondition(MatTestNeumannBC);

  registerBoundaryCondition(DGMDDBC);
  registerBoundaryCondition(DGFunctionConvectionDirichletBC);

  registerBoundaryCondition(CoupledKernelGradBC);

  registerBoundaryCondition(DivergenceBC);
  registerBoundaryCondition(MatDivergenceBC);
  registerBoundaryCondition(CoupledDirichletBC);
  registerBoundaryCondition(TestLapBC);

  // Initial conditions
  registerInitialCondition(TEIC);
  registerInitialCondition(MTICSum);
  registerInitialCondition(MTICMult);
  registerInitialCondition(DataStructIC);

  // Materials
  registerMaterial(MTMaterial);
  registerMaterial(TypesMaterial);
  registerMaterial(StatefulMaterial);
  registerMaterial(SpatialStatefulMaterial);
  registerMaterial(ComputingInitialTest);
  registerMaterial(StatefulTest);
  registerMaterial(StatefulSpatialTest);
  registerMaterial(CoupledMaterial);
  registerMaterial(CoupledMaterial2);
  registerMaterial(LinearInterpolationMaterial);
  registerMaterial(VarCouplingMaterial);
  registerMaterial(VarCouplingMaterialEigen);
  registerMaterial(BadStatefulMaterial);
  registerMaterial(OutputTestMaterial);
  registerMaterial(SumMaterial);
  registerMaterial(VecRangeCheckMaterial);
  registerMaterial(DerivativeMaterialInterfaceTestProvider);
  registerMaterial(DerivativeMaterialInterfaceTestClient);
  registerMaterial(DefaultMatPropConsumerMaterial);
  registerMaterial(RandomMaterial);
  registerMaterial(RecomputeMaterial);
  registerMaterial(NewtonMaterial);
  registerMaterial(ThrowMaterial);


  registerScalarKernel(ExplicitODE);
  registerScalarKernel(ImplicitODEx);
  registerScalarKernel(ImplicitODEy);
  registerScalarKernel(AlphaCED);
  registerScalarKernel(PostprocessorCED);
  registerScalarKernel(VectorPostprocessorScalarKernel);

  // Functions
  registerFunction(TimestepSetupFunction);
  registerFunction(PostprocessorFunction);
  registerFunction(MTPiecewiseConst1D);
  registerFunction(MTPiecewiseConst2D);
  registerFunction(MTPiecewiseConst3D);
  registerFunction(TestSetupPostprocessorDataActionFunction);

  // DiracKernels
  registerDiracKernel(ReportingConstantSource);
  registerDiracKernel(FrontSource);
  registerDiracKernel(MaterialPointSource);
  registerDiracKernel(MaterialMultiPointSource);
  registerDiracKernel(CachingPointSource);
  registerDiracKernel(BadCachingPointSource);
  registerDiracKernel(NonlinearSource);

  // meshes
  registerObject(StripeMesh);

  registerConstraint(EqualValueNodalConstraint);

  // UserObjects
  registerUserObject(MTUserObject);
  registerUserObject(RandomHitUserObject);
  registerUserObject(RandomHitSolutionModifier);
  registerUserObject(MaterialPropertyUserObject);
  registerUserObject(MaterialCopyUserObject);
  registerUserObject(InsideUserObject);
  registerUserObject(RestartableTypes);
  registerUserObject(RestartableTypesChecker);
  registerUserObject(PointerStoreError);
  registerUserObject(PointerLoadError);
  registerUserObject(VerifyElementUniqueID);
  registerUserObject(VerifyNodalUniqueID);
  registerUserObject(RandomElementalUserObject);
  registerUserObject(TrackDiracFront);
  registerUserObject(BoundaryUserObject);
  registerUserObject(TestBoundaryRestrictableAssert);
  registerUserObject(GetMaterialPropertyBoundaryBlockNamesTest);
  registerUserObject(GeneralSetupInterfaceCount);
  registerUserObject(ElementSetupInterfaceCount);
  registerUserObject(SideSetupInterfaceCount);
  registerUserObject(InternalSideSetupInterfaceCount);
  registerUserObject(NodalSetupInterfaceCount);
  registerUserObject(ReadDoubleIndex);
  registerUserObject(TestShapeElementUserObject);
  registerUserObject(ExampleShapeElementUserObject);
  registerUserObject(SimpleTestShapeElementUserObject);

  registerPostprocessor(InsideValuePPS);
  registerPostprocessor(TestCopyInitialSolution);
  registerPostprocessor(TestSerializedSolution);
  registerPostprocessor(BoundaryValuePPS);
  registerPostprocessor(NumInternalSides);
  registerPostprocessor(NumElemQPs);
  registerPostprocessor(NumSideQPs);
  registerPostprocessor(ElementL2Diff);
  registerPostprocessor(TestPostprocessor);
  registerPostprocessor(ElementSidePP);
  registerPostprocessor(RealControlParameterReporter);
  registerPostprocessor(ScalarCoupledPostprocessor);
  registerPostprocessor(NumAdaptivityCycles);

  registerMarker(RandomHitMarker);
  registerMarker(QPointMarker);
  registerMarker(CircleMarker);

  registerExecutioner(TestSteady);
  registerExecutioner(AdaptAndModify);

  registerProblem(MooseTestProblem);
  registerProblem(FailingProblem);

  // TimeSteppers
  registerTimeStepper(TimeSequenceStepperFailTest);

  // Outputs
  registerOutput(OutputObjectTest);

  // Controls
  registerControl(TestControl);

  // Indicators
  registerIndicator(MaterialTestIndicator);
}

// External entry point for dynamic syntax association
extern "C" void MooseTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { MooseTestApp::associateSyntax(syntax, action_factory); }
void
MooseTestApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  // and add more
  registerAction(ConvDiffMetaAction, "meta_action");
  registerAction(AddLotsOfAuxVariablesAction, "meta_action");
  registerAction(BadAddKernelAction, "add_kernel");

  registerAction(AddLotsOfDiffusion, "add_variable");
  registerAction(AddLotsOfDiffusion, "add_kernel");
  registerAction(AddLotsOfDiffusion, "add_bc");

  registerAction(TestGetActionsAction, "meta_action");

  syntax.registerActionSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
  syntax.registerActionSyntax("AddAuxVariableAction", "MoreAuxVariables/*", "add_aux_variable");
  syntax.registerActionSyntax("AddLotsOfAuxVariablesAction", "LotsOfAuxVariables/*", "add_variable");

  registerAction(ApplyCoupledVariablesTestAction, "meta_action");
  syntax.registerActionSyntax("ApplyCoupledVariablesTestAction", "ApplyInputParametersTest");
  syntax.registerActionSyntax("AddLotsOfDiffusion", "Testing/LotsOfDiffusion/*");
  syntax.registerActionSyntax("TestGetActionsAction", "TestGetActions");
  syntax.registerActionSyntax("BadAddKernelAction", "BadKernels/*");
}
