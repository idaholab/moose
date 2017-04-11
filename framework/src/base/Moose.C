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

#include "libmesh/petsc_macro.h"
#include "libmesh/libmesh_config.h"

#include "Moose.h"

#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "AuxiliarySystem.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "Syntax.h"

// objects that can be created by MOOSE
// Mesh
#include "FileMesh.h"
#include "GeneratedMesh.h"
#include "TiledMesh.h"
#include "ImageMesh.h"
#include "PatternedMesh.h"
#include "StitchedMesh.h"

// MeshModifiers
#include "MeshExtruder.h"
#include "SideSetsFromPoints.h"
#include "SideSetsFromNormals.h"
#include "AddExtraNodeset.h"
#include "BoundingBoxNodeSet.h"
#include "Transform.h"
#include "SideSetsAroundSubdomain.h"
#include "SideSetsBetweenSubdomains.h"
#include "AddAllSideSetsByNormals.h"
#include "SubdomainBoundingBox.h"
#include "OrientedSubdomainBoundingBox.h"
#include "RenameBlock.h"
#include "AssignElementSubdomainID.h"
#include "ImageSubdomain.h"
#include "BlockDeleter.h"
#include "ParsedSubdomainMeshModifier.h"
#include "BreakBoundaryOnSubdomain.h"
#include "ParsedAddSideset.h"
#include "AssignSubdomainID.h"

// problems
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "EigenProblem.h"

// kernels
#include "ConservativeAdvection.h"
#include "TimeDerivative.h"
#include "CoupledTimeDerivative.h"
#include "MassLumpedTimeDerivative.h"
#include "Diffusion.h"
#include "AnisotropicDiffusion.h"
#include "CoupledForce.h"
#include "UserForcingFunction.h"
#include "BodyForce.h"
#include "Reaction.h"
#include "MassEigenKernel.h"
#include "NullKernel.h"
#include "MaterialDerivativeTestKernel.h"

// bcs
#include "ConvectiveFluxBC.h"
#include "DirichletBC.h"
#include "PenaltyDirichletBC.h"
#include "PresetBC.h"
#include "NeumannBC.h"
#include "PostprocessorNeumannBC.h"
#include "FunctionDirichletBC.h"
#include "FunctionPenaltyDirichletBC.h"
#include "FunctionPresetBC.h"
#include "FunctionNeumannBC.h"
#include "MatchedValueBC.h"
#include "VacuumBC.h"
#include "SinDirichletBC.h"
#include "SinNeumannBC.h"
#include "VectorNeumannBC.h"
#include "WeakGradientBC.h"
#include "DiffusionFluxBC.h"
#include "PostprocessorDirichletBC.h"
#include "OneDEqualValueConstraintBC.h"

// auxkernels
#include "ConstantAux.h"
#include "FunctionAux.h"
#include "NearestNodeDistanceAux.h"
#include "NearestNodeValueAux.h"
#include "PenetrationAux.h"
#include "ProcessorIDAux.h"
#include "SelfAux.h"
#include "GapValueAux.h"
#include "MaterialRealAux.h"
#include "MaterialRealVectorValueAux.h"
#include "MaterialRealTensorValueAux.h"
#include "MaterialStdVectorAux.h"
#include "MaterialRealDenseMatrixAux.h"
#include "MaterialStdVectorRealGradientAux.h"
#include "DebugResidualAux.h"
#include "BoundsAux.h"
#include "SpatialUserObjectAux.h"
#include "SolutionAux.h"
#include "VectorMagnitudeAux.h"
#include "ConstantScalarAux.h"
#include "QuotientAux.h"
#include "NormalizationAux.h"
#include "VariableGradientComponent.h"
#include "ParsedAux.h"
#include "VariableTimeIntegrationAux.h"
#include "ElementLengthAux.h"
#include "ElementLpNormAux.h"
#include "ElementL2ErrorFunctionAux.h"
#include "ElementH1ErrorFunctionAux.h"

// dirac kernels
#include "ConstantPointSource.h"

// DG
#include "DGDiffusion.h"
#include "DGFunctionDiffusionDirichletBC.h"
#include "DGConvection.h"

// ics
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "FunctionIC.h"
#include "RandomIC.h"
#include "ScalarConstantIC.h"
#include "ScalarComponentIC.h"
#include "FunctionScalarIC.h"

// executioners
#include "Steady.h"
#include "Transient.h"
#include "InversePowerMethod.h"
#include "NonlinearEigen.h"

// functions
#include "Axisymmetric2D3DSolutionFunction.h"
#include "ConstantFunction.h"
#include "CompositeFunction.h"
#include "MooseParsedFunction.h"
#include "MooseParsedVectorFunction.h"
#include "MooseParsedGradFunction.h"
#include "PiecewiseConstant.h"
#include "PiecewiseLinear.h"
#include "SolutionFunction.h"
#include "PiecewiseBilinear.h"
#include "SplineFunction.h"
#include "BicubicSplineFunction.h"
#include "PiecewiseMultilinear.h"
#include "LinearCombinationFunction.h"
#include "ImageFunction.h"
#include "VectorPostprocessorFunction.h"

// materials
#include "GenericConstantMaterial.h"
#include "GenericConstantRankTwoTensor.h"
#include "GenericFunctionMaterial.h"
#include "PiecewiseLinearInterpolationMaterial.h"

// PPS
#include "AverageElementSize.h"
#include "AverageNodalVariableValue.h"
#include "CumulativeValuePostprocessor.h"
#include "ChangeOverTimestepPostprocessor.h"
#include "NodalSum.h"
#include "ElementAverageValue.h"
#include "ElementAverageTimeDerivative.h"
#include "ElementW1pError.h"
#include "ElementH1Error.h"
#include "ElementH1SemiError.h"
#include "ElementIntegralVariablePostprocessor.h"
#include "ElementIntegralMaterialProperty.h"
#include "ElementL2Error.h"
#include "ElementVectorL2Error.h"
#include "EmptyPostprocessor.h"
#include "FindValueOnLine.h"
#include "FunctionValuePostprocessor.h"
#include "NodalVariableValue.h"
#include "NumDOFs.h"
#include "TimestepSize.h"
#include "RunTime.h"
#include "PerformanceData.h"
#include "MemoryUsage.h"
#include "NumElems.h"
#include "NumNodes.h"
#include "NumNonlinearIterations.h"
#include "NumLinearIterations.h"
#include "Residual.h"
#include "ScalarVariable.h"
#include "NumVars.h"
#include "NumResidualEvaluations.h"
#include "Receiver.h"
#include "SideAverageValue.h"
#include "SideFluxIntegral.h"
#include "SideFluxAverage.h"
#include "SideIntegralVariablePostprocessor.h"
#include "NodalMaxValue.h"
#include "NodalProxyMaxValue.h"
#include "ScalarL2Error.h"
#include "ElementalVariableValue.h"
#include "ElementL2Norm.h"
#include "NodalL2Norm.h"
#include "NodalL2Error.h"
#include "TotalVariableValue.h"
#include "VolumePostprocessor.h"
#include "AreaPostprocessor.h"
#include "PointValue.h"
#include "NodalExtremeValue.h"
#include "ElementExtremeValue.h"
#include "DifferencePostprocessor.h"
#include "ScalePostprocessor.h"
#include "NumPicardIterations.h"
#include "FunctionSideIntegral.h"
#include "ExecutionerAttributeReporter.h"
#include "PercentChangePostprocessor.h"
#include "ElementL2Difference.h"
#include "TimeExtremeValue.h"
#include "RelativeSolutionDifferenceNorm.h"
#include "AxisymmetricCenterlineAverageValue.h"
#include "VariableInnerProduct.h"
#include "VariableResidual.h"

// vector PPS
#include "MaterialVectorPostprocessor.h"
#include "ConstantVectorPostprocessor.h"
#include "Eigenvalues.h"
#include "NodalValueSampler.h"
#include "SideValueSampler.h"
#include "PointValueSampler.h"
#include "LineValueSampler.h"
#include "VectorOfPostprocessors.h"
#include "LeastSquaresFit.h"
#include "ElementsAlongLine.h"
#include "ElementsAlongPlane.h"
#include "IntersectionPointsAlongLine.h"
#include "LineMaterialRealSampler.h"
#include "LineFunctionSampler.h"
#include "VolumeHistogram.h"
#include "SphericalAverage.h"

// user objects
#include "LayeredIntegral.h"
#include "LayeredAverage.h"
#include "LayeredSideIntegral.h"
#include "LayeredSideAverage.h"
#include "LayeredSideFluxAverage.h"
#include "NearestPointLayeredAverage.h"
#include "ElementIntegralVariableUserObject.h"
#include "NodalNormalsEvaluator.h"
#include "NodalNormalsCorner.h"
#include "NodalNormalsPreprocessor.h"
#include "SolutionUserObject.h"
#ifdef LIBMESH_HAVE_FPARSER
#include "Terminator.h"
#endif

// preconditioners
#include "PhysicsBasedPreconditioner.h"
#include "FiniteDifferencePreconditioner.h"
#include "SingleMatrixPreconditioner.h"

#include "FieldSplitPreconditioner.h"
#include "Split.h"
#include "AddFieldSplitAction.h"

// dampers
#include "ConstantDamper.h"
#include "MaxIncrement.h"
#include "BoundingValueNodalDamper.h"
#include "BoundingValueElementDamper.h"

// Constraints
#include "TiedValueConstraint.h"
#include "CoupledTiedValueConstraint.h"
#include "AddBoundsVectorsAction.h"
#include "EqualGradientConstraint.h"
#include "EqualValueConstraint.h"
#include "EqualValueBoundaryConstraint.h"
#include "LinearNodalConstraint.h"

// ScalarKernels
#include "ODETimeDerivative.h"
#include "FunctionScalarAux.h"
#include "NodalEqualValueConstraint.h"
#include "ParsedODEKernel.h"
#include "QuotientScalarAux.h"

// indicators
#include "AnalyticalIndicator.h"
#include "LaplacianJumpIndicator.h"
#include "GradientJumpIndicator.h"

// markers
#include "ErrorToleranceMarker.h"
#include "ErrorFractionMarker.h"
#include "UniformMarker.h"
#include "BoxMarker.h"
#include "ComboMarker.h"
#include "ValueThresholdMarker.h"
#include "ValueRangeMarker.h"
#include "OrientedBoxMarker.h"

// time steppers
#include "ConstantDT.h"
#include "FunctionDT.h"
#include "TimeSequenceStepper.h"
#include "ExodusTimeSequenceStepper.h"
#include "IterationAdaptiveDT.h"
#include "SolutionTimeAdaptiveDT.h"
#include "DT2.h"
#include "PostprocessorDT.h"
#include "AB2PredictorCorrector.h"

// time integrators
#include "SteadyState.h"
#include "ImplicitEuler.h"
#include "BDF2.h"
#include "CrankNicolson.h"
#include "ExplicitEuler.h"
#include "ExplicitMidpoint.h"
#include "ExplicitTVDRK2.h"
#include "LStableDirk2.h"
#include "LStableDirk3.h"
#include "AStableDirk4.h"
#include "LStableDirk4.h"
#include "ImplicitMidpoint.h"
#include "Heun.h"
#include "Ralston.h"
#include "SimplePredictor.h"
#include "AdamsPredictor.h"

// MultiApps
#include "TransientMultiApp.h"
#include "FullSolveMultiApp.h"
#include "AutoPositionsMultiApp.h"

// Transfers
#ifdef LIBMESH_TRILINOS_HAVE_DTK
#include "MultiAppDTKUserObjectTransfer.h"
#include "MultiAppDTKInterpolationTransfer.h"
#endif
#include "MultiAppPostprocessorInterpolationTransfer.h"
#include "MultiAppVariableValueSampleTransfer.h"
#include "MultiAppVariableValueSamplePostprocessorTransfer.h"
#include "MultiAppMeshFunctionTransfer.h"
#include "MultiAppUserObjectTransfer.h"
#include "MultiAppNearestNodeTransfer.h"
#include "MultiAppCopyTransfer.h"
#include "MultiAppInterpolationTransfer.h"
#include "MultiAppPostprocessorTransfer.h"
#include "MultiAppProjectionTransfer.h"
#include "MultiAppPostprocessorToAuxScalarTransfer.h"

// Actions
#include "AddBCAction.h"
#include "AddDiracKernelAction.h"
#include "AddICAction.h"
#include "AddInitialConditionAction.h"
#include "AddKernelAction.h"
#include "AddScalarKernelAction.h"
#include "AddDGKernelAction.h"
#include "AddInterfaceKernelAction.h"
#include "AddPeriodicBCAction.h"
#include "AddVariableAction.h"
#include "AddAuxVariableAction.h"
#include "AddPostprocessorAction.h"
#include "AddVectorPostprocessorAction.h"
#include "AddDamperAction.h"
#include "AddFunctionAction.h"
#include "AddDistributionAction.h"
#include "CreateExecutionerAction.h"
#include "DetermineSystemType.h"
#include "EmptyAction.h"
#include "InitProblemAction.h"
#include "CopyNodalVarsAction.h"
#include "SetupMeshAction.h"
#include "AddMeshModifierAction.h"
#include "SetupMeshCompleteAction.h"
#include "AddOutputAction.h"
#include "CommonOutputAction.h"
#include "AddMaterialAction.h"
#include "GlobalParamsAction.h"
#include "AdaptivityAction.h"
#include "PartitionerAction.h"
#include "SetupDampersAction.h"
#include "CheckIntegrityAction.h"
#include "SetupQuadratureAction.h"
#include "SetupPreconditionerAction.h"
#include "SetupDebugAction.h"
#include "SetupResidualDebugAction.h"
#include "DeprecatedBlockAction.h"
#include "AddConstraintAction.h"
#include "CreateDisplacedProblemAction.h"
#include "CreateProblemAction.h"
#include "DynamicObjectRegistrationAction.h"
#include "AddUserObjectAction.h"
#include "AddControlAction.h"
#include "AddElementalFieldAction.h"
#include "AddIndicatorAction.h"
#include "AddMarkerAction.h"
#include "SetAdaptivityOptionsAction.h"
#include "AddMultiAppAction.h"
#include "AddTransferAction.h"
#include "AddNodalNormalsAction.h"
#include "SetupTimeStepperAction.h"
#include "SetupTimeIntegratorAction.h"
#include "SetupPredictorAction.h"
#include "AddMortarInterfaceAction.h"
#include "SetupPostprocessorDataAction.h"
#include "MaterialOutputAction.h"
#include "CheckOutputAction.h"
#include "SetupRecoverFileBaseAction.h"
#include "AddNodalKernelAction.h"

// Outputs
#ifdef LIBMESH_HAVE_EXODUS_API
#include "Exodus.h"
#endif
#include "Nemesis.h"
#include "Console.h"
#include "CSV.h"
#include "VTKOutput.h"
#include "Checkpoint.h"
#include "XDA.h"
#include "GMVOutput.h"
#include "Tecplot.h"
#include "Gnuplot.h"
#include "SolutionHistory.h"
#include "MaterialPropertyDebugOutput.h"
#include "VariableResidualNormsDebugOutput.h"
#include "TopResidualDebugOutput.h"
#include "DOFMapOutput.h"
#include "ControlOutput.h"

// Controls
#include "RealFunctionControl.h"
#include "TimePeriod.h"

// Partitioner
#include "LibmeshPartitioner.h"

// NodalKernels
#include "ConstantRate.h"
#include "TimeDerivativeNodalKernel.h"
#include "UserForcingFunctionNodalKernel.h"

#include <unistd.h>

const ExecFlagType EXEC_NONE = 0x00;           // 0
const ExecFlagType EXEC_INITIAL = 0x01;        // 1
const ExecFlagType EXEC_LINEAR = 0x02;         // 2
const ExecFlagType EXEC_NONLINEAR = 0x04;      // 4
const ExecFlagType EXEC_TIMESTEP_END = 0x8;    // 8
const ExecFlagType EXEC_TIMESTEP_BEGIN = 0x10; // 16
const ExecFlagType EXEC_FINAL = 0x20;          // 32
const ExecFlagType EXEC_FORCED = 0x40;         // 64
const ExecFlagType EXEC_FAILED = 0x80;         // 128
const ExecFlagType EXEC_CUSTOM = 0x100;        // 256
const ExecFlagType EXEC_SUBDOMAIN = 0x200;     // 512
const ExecFlagType EXEC_SAME_AS_MULTIAPP = 1000;

namespace Moose
{

static bool registered = false;

void
registerObjects(Factory & factory)
{
  // mesh
  registerMesh(FileMesh);
  registerMesh(GeneratedMesh);
  registerMesh(TiledMesh);
  registerMesh(ImageMesh);
  registerMesh(PatternedMesh);
  registerMesh(StitchedMesh);

  // mesh modifiers
  registerMeshModifier(MeshExtruder);
  registerMeshModifier(SideSetsFromPoints);
  registerMeshModifier(SideSetsFromNormals);
  registerMeshModifier(AddExtraNodeset);
  registerMeshModifier(BoundingBoxNodeSet);
  registerMeshModifier(Transform);
  registerMeshModifier(SideSetsAroundSubdomain);
  registerMeshModifier(SideSetsBetweenSubdomains);
  registerMeshModifier(AddAllSideSetsByNormals);
  registerMeshModifier(SubdomainBoundingBox);
  registerMeshModifier(OrientedSubdomainBoundingBox);
  registerMeshModifier(RenameBlock);
  registerMeshModifier(AssignElementSubdomainID);
  registerMeshModifier(ImageSubdomain);
  registerMeshModifier(BlockDeleter);
  registerMeshModifier(ParsedSubdomainMeshModifier);
  registerMeshModifier(BreakBoundaryOnSubdomain);
  registerMeshModifier(ParsedAddSideset);
  registerMeshModifier(AssignSubdomainID);

  // problems
  registerProblem(DisplacedProblem);
  registerProblem(FEProblem);
  registerProblem(EigenProblem);

  // kernels
  registerKernel(TimeDerivative);
  registerKernel(ConservativeAdvection);
  registerKernel(CoupledTimeDerivative);
  registerKernel(MassLumpedTimeDerivative);
  registerKernel(Diffusion);
  registerKernel(AnisotropicDiffusion);
  registerKernel(CoupledForce);
  registerKernel(UserForcingFunction);
  registerKernel(BodyForce);
  registerKernel(Reaction);
  registerKernel(MassEigenKernel);
  registerKernel(NullKernel);
  registerKernel(MaterialDerivativeTestKernel);

  // bcs
  registerBoundaryCondition(ConvectiveFluxBC);
  registerBoundaryCondition(DirichletBC);
  registerBoundaryCondition(PenaltyDirichletBC);
  registerBoundaryCondition(PresetBC);
  registerBoundaryCondition(NeumannBC);
  registerBoundaryCondition(PostprocessorNeumannBC);
  registerBoundaryCondition(FunctionDirichletBC);
  registerBoundaryCondition(FunctionPenaltyDirichletBC);
  registerBoundaryCondition(FunctionPresetBC);
  registerBoundaryCondition(FunctionNeumannBC);
  registerBoundaryCondition(MatchedValueBC);
  registerBoundaryCondition(VacuumBC);

  registerBoundaryCondition(SinDirichletBC);
  registerBoundaryCondition(SinNeumannBC);
  registerBoundaryCondition(VectorNeumannBC);
  registerBoundaryCondition(WeakGradientBC);
  registerBoundaryCondition(DiffusionFluxBC);
  registerBoundaryCondition(PostprocessorDirichletBC);
  registerBoundaryCondition(OneDEqualValueConstraintBC);

  // dirac kernels
  registerDiracKernel(ConstantPointSource);

  // aux kernels
  registerAux(ConstantAux);
  registerAux(FunctionAux);
  registerAux(NearestNodeDistanceAux);
  registerAux(NearestNodeValueAux);
  registerAux(PenetrationAux);
  registerAux(ProcessorIDAux);
  registerAux(SelfAux);
  registerAux(GapValueAux);
  registerAux(MaterialRealAux);
  registerAux(MaterialRealVectorValueAux);
  registerAux(MaterialRealTensorValueAux);
  registerAux(MaterialStdVectorAux);
  registerAux(MaterialRealDenseMatrixAux);
  registerAux(MaterialStdVectorRealGradientAux);
  registerAux(DebugResidualAux);
  registerAux(BoundsAux);
  registerAux(SpatialUserObjectAux);
  registerAux(SolutionAux);
  registerAux(VectorMagnitudeAux);
  registerAux(ConstantScalarAux);
  registerAux(QuotientAux);
  registerAux(NormalizationAux);
  registerAux(FunctionScalarAux);
  registerAux(VariableGradientComponent);
  registerAux(ParsedAux);
  registerAux(VariableTimeIntegrationAux);
  registerAux(ElementLengthAux);
  registerAux(ElementLpNormAux);
  registerAux(ElementL2ErrorFunctionAux);
  registerAux(ElementH1ErrorFunctionAux);

  // Initial Conditions
  registerInitialCondition(ConstantIC);
  registerInitialCondition(BoundingBoxIC);
  registerInitialCondition(FunctionIC);
  registerInitialCondition(RandomIC);
  registerInitialCondition(ScalarConstantIC);
  registerInitialCondition(ScalarComponentIC);
  registerInitialCondition(FunctionScalarIC);

  // executioners
  registerExecutioner(Steady);
  registerExecutioner(Transient);
  registerExecutioner(InversePowerMethod);
  registerExecutioner(NonlinearEigen);

  // functions
  registerFunction(Axisymmetric2D3DSolutionFunction);
  registerFunction(ConstantFunction);
  registerFunction(CompositeFunction);
  registerNamedFunction(MooseParsedFunction, "ParsedFunction");
  registerNamedFunction(MooseParsedGradFunction, "ParsedGradFunction");
  registerNamedFunction(MooseParsedVectorFunction, "ParsedVectorFunction");
  registerFunction(PiecewiseConstant);
  registerFunction(PiecewiseLinear);
  registerFunction(SolutionFunction);
  registerFunction(PiecewiseBilinear);
  registerFunction(SplineFunction);
  registerFunction(BicubicSplineFunction);
  registerFunction(PiecewiseMultilinear);
  registerFunction(LinearCombinationFunction);
  registerFunction(ImageFunction);
  registerFunction(VectorPostprocessorFunction);

  // materials
  registerMaterial(GenericConstantMaterial);
  registerMaterial(GenericConstantRankTwoTensor);
  registerMaterial(GenericFunctionMaterial);
  registerMaterial(PiecewiseLinearInterpolationMaterial);

  // PPS
  registerPostprocessor(AverageElementSize);
  registerPostprocessor(AverageNodalVariableValue);
  registerPostprocessor(CumulativeValuePostprocessor);
  registerPostprocessor(ChangeOverTimestepPostprocessor);
  registerPostprocessor(NodalSum);
  registerPostprocessor(ElementAverageValue);
  registerPostprocessor(ElementAverageTimeDerivative);
  registerPostprocessor(ElementW1pError);
  registerPostprocessor(ElementH1Error);
  registerPostprocessor(ElementH1SemiError);
  registerPostprocessor(ElementIntegralVariablePostprocessor);
  registerPostprocessor(ElementIntegralMaterialProperty);
  registerPostprocessor(ElementL2Error);
  registerPostprocessor(ElementVectorL2Error);
  registerPostprocessor(ScalarL2Error);
  registerPostprocessor(EmptyPostprocessor);
  registerPostprocessor(FindValueOnLine);
  registerPostprocessor(NodalVariableValue);
  registerPostprocessor(NumDOFs);
  registerPostprocessor(TimestepSize);
  registerPostprocessor(RunTime);
  registerPostprocessor(PerformanceData);
  registerPostprocessor(MemoryUsage);
  registerPostprocessor(NumElems);
  registerPostprocessor(NumNodes);
  registerPostprocessor(NumNonlinearIterations);
  registerPostprocessor(NumLinearIterations);
  registerPostprocessor(Residual);
  registerPostprocessor(ScalarVariable);
  registerPostprocessor(NumVars);
  registerPostprocessor(NumResidualEvaluations);
  registerPostprocessor(Receiver);
  registerPostprocessor(SideAverageValue);
  registerPostprocessor(SideFluxIntegral);
  registerPostprocessor(SideFluxAverage);
  registerPostprocessor(SideIntegralVariablePostprocessor);
  registerPostprocessor(NodalMaxValue);
  registerPostprocessor(NodalProxyMaxValue);
  registerPostprocessor(ElementalVariableValue);
  registerPostprocessor(ElementL2Norm);
  registerPostprocessor(NodalL2Norm);
  registerPostprocessor(NodalL2Error);
  registerPostprocessor(TotalVariableValue);
  registerPostprocessor(VolumePostprocessor);
  registerPostprocessor(AreaPostprocessor);
  registerPostprocessor(PointValue);
  registerPostprocessor(NodalExtremeValue);
  registerPostprocessor(ElementExtremeValue);
  registerPostprocessor(DifferencePostprocessor);
  registerPostprocessor(ScalePostprocessor);
  registerPostprocessor(FunctionValuePostprocessor);
  registerPostprocessor(NumPicardIterations);
  registerPostprocessor(FunctionSideIntegral);
  registerPostprocessor(ExecutionerAttributeReporter);
  registerPostprocessor(PercentChangePostprocessor);
  registerPostprocessor(ElementL2Difference);
  registerPostprocessor(TimeExtremeValue);
  registerPostprocessor(RelativeSolutionDifferenceNorm);
  registerPostprocessor(AxisymmetricCenterlineAverageValue);
  registerPostprocessor(VariableInnerProduct);
  registerPostprocessor(VariableResidual);

  // vector PPS
  registerVectorPostprocessor(ConstantVectorPostprocessor);
  registerVectorPostprocessor(MaterialVectorPostprocessor);
  registerVectorPostprocessor(Eigenvalues);
  registerVectorPostprocessor(NodalValueSampler);
  registerVectorPostprocessor(SideValueSampler);
  registerVectorPostprocessor(PointValueSampler);
  registerVectorPostprocessor(LineValueSampler);
  registerVectorPostprocessor(VectorOfPostprocessors);
  registerVectorPostprocessor(LeastSquaresFit);
  registerVectorPostprocessor(ElementsAlongLine);
  registerVectorPostprocessor(ElementsAlongPlane);
  registerVectorPostprocessor(IntersectionPointsAlongLine);
  registerVectorPostprocessor(LineMaterialRealSampler);
  registerVectorPostprocessor(LineFunctionSampler);
  registerVectorPostprocessor(VolumeHistogram);
  registerVectorPostprocessor(SphericalAverage);

  // user objects
  registerUserObject(LayeredIntegral);
  registerUserObject(LayeredAverage);
  registerUserObject(LayeredSideIntegral);
  registerUserObject(LayeredSideAverage);
  registerUserObject(LayeredSideFluxAverage);
  registerUserObject(NearestPointLayeredAverage);
  registerUserObject(ElementIntegralVariableUserObject);
  registerUserObject(NodalNormalsPreprocessor);
  registerUserObject(NodalNormalsCorner);
  registerUserObject(NodalNormalsEvaluator);
  registerUserObject(SolutionUserObject);
#ifdef LIBMESH_HAVE_FPARSER
  registerUserObject(Terminator);
#endif

  // preconditioners
  registerNamedPreconditioner(PhysicsBasedPreconditioner, "PBP");
  registerNamedPreconditioner(FiniteDifferencePreconditioner, "FDP");
  registerNamedPreconditioner(SingleMatrixPreconditioner, "SMP");
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3, 3, 0)
  registerNamedPreconditioner(FieldSplitPreconditioner, "FSP");
#endif
  // dampers
  registerDamper(ConstantDamper);
  registerDamper(MaxIncrement);
  registerDamper(BoundingValueNodalDamper);
  registerDamper(BoundingValueElementDamper);
  // DG
  registerDGKernel(DGDiffusion);
  registerBoundaryCondition(DGFunctionDiffusionDirichletBC);
  registerDGKernel(DGConvection);

  // Constraints
  registerConstraint(TiedValueConstraint);
  registerConstraint(CoupledTiedValueConstraint);
  registerConstraint(EqualGradientConstraint);
  registerConstraint(EqualValueConstraint);
  registerConstraint(EqualValueBoundaryConstraint);
  registerConstraint(LinearNodalConstraint);

  // Scalar kernels
  registerScalarKernel(ODETimeDerivative);
  registerScalarKernel(NodalEqualValueConstraint);
  registerScalarKernel(ParsedODEKernel);
  registerScalarKernel(QuotientScalarAux);

  // indicators
  registerIndicator(AnalyticalIndicator);
  registerIndicator(LaplacianJumpIndicator);
  registerIndicator(GradientJumpIndicator);

  // markers
  registerMarker(ErrorToleranceMarker);
  registerMarker(ErrorFractionMarker);
  registerMarker(UniformMarker);
  registerMarker(BoxMarker);
  registerMarker(OrientedBoxMarker);
  registerMarker(ComboMarker);
  registerMarker(ValueThresholdMarker);
  registerMarker(ValueRangeMarker);

  // splits
  registerSplit(Split);

  // MultiApps
  registerMultiApp(TransientMultiApp);
  registerMultiApp(FullSolveMultiApp);
  registerMultiApp(AutoPositionsMultiApp);

  // time steppers
  registerTimeStepper(ConstantDT);
  registerTimeStepper(FunctionDT);
  registerTimeStepper(TimeSequenceStepper);
  registerTimeStepper(ExodusTimeSequenceStepper);
  registerTimeStepper(IterationAdaptiveDT);
  registerTimeStepper(SolutionTimeAdaptiveDT);
  registerTimeStepper(DT2);
  registerTimeStepper(PostprocessorDT);
  registerTimeStepper(AB2PredictorCorrector);
  // time integrators
  registerTimeIntegrator(SteadyState);
  registerTimeIntegrator(ImplicitEuler);
  registerTimeIntegrator(BDF2);
  registerTimeIntegrator(CrankNicolson);
  registerTimeIntegrator(ExplicitEuler);
  registerTimeIntegrator(ExplicitMidpoint);
  registerTimeIntegrator(ExplicitTVDRK2);
  registerTimeIntegrator(LStableDirk2);
  registerTimeIntegrator(LStableDirk3);
  registerTimeIntegrator(AStableDirk4);
  registerTimeIntegrator(LStableDirk4);
  registerTimeIntegrator(ImplicitMidpoint);
  registerTimeIntegrator(Heun);
  registerTimeIntegrator(Ralston);
  // predictors
  registerPredictor(SimplePredictor);
  registerPredictor(AdamsPredictor);

// Transfers
#ifdef LIBMESH_TRILINOS_HAVE_DTK
  registerTransfer(MultiAppDTKUserObjectTransfer);
  registerTransfer(MultiAppDTKInterpolationTransfer);
#endif
  registerTransfer(MultiAppPostprocessorInterpolationTransfer);
  registerTransfer(MultiAppVariableValueSampleTransfer);
  registerTransfer(MultiAppVariableValueSamplePostprocessorTransfer);
  registerTransfer(MultiAppMeshFunctionTransfer);
  registerTransfer(MultiAppUserObjectTransfer);
  registerTransfer(MultiAppNearestNodeTransfer);
  registerTransfer(MultiAppCopyTransfer);
  registerTransfer(MultiAppInterpolationTransfer);
  registerTransfer(MultiAppPostprocessorTransfer);
  registerTransfer(MultiAppProjectionTransfer);
  registerTransfer(MultiAppPostprocessorToAuxScalarTransfer);

// Outputs
#ifdef LIBMESH_HAVE_EXODUS_API
  registerOutput(Exodus);
#endif
#ifdef LIBMESH_HAVE_NEMESIS_API
  registerOutput(Nemesis);
#endif
  registerOutput(Console);
  registerOutput(CSV);
#ifdef LIBMESH_HAVE_VTK
  registerNamedOutput(VTKOutput, "VTK");
#endif
  registerOutput(Checkpoint);
  registerNamedOutput(XDA, "XDR");
  registerOutput(XDA);
  registerNamedOutput(GMVOutput, "GMV");
  registerOutput(Tecplot);
  registerOutput(Gnuplot);
  registerOutput(SolutionHistory);
  registerOutput(MaterialPropertyDebugOutput);
  registerOutput(VariableResidualNormsDebugOutput);
  registerOutput(TopResidualDebugOutput);
  registerNamedOutput(DOFMapOutput, "DOFMap");
  registerOutput(ControlOutput);

  // Controls
  registerControl(RealFunctionControl);
  registerControl(TimePeriod);

  // Partitioner
  registerPartitioner(LibmeshPartitioner);

  // NodalKernels
  registerNodalKernel(TimeDerivativeNodalKernel);
  registerNodalKernel(ConstantRate);
  registerNodalKernel(UserForcingFunctionNodalKernel);

  registered = true;
}

void
addActionTypes(Syntax & syntax)
{
  /**
   * The second param here indicates whether the task must be satisfied or not for a successful run.
   * If set to true, then the ActionWarehouse will attempt to create "Action"s automatically if they
   * have
   * not been explicitly created by the parser or some other mechanism.
   *
   * Note: Many of the actions in the "Minimal Problem" section are marked as false.  However, we
   * can generally
   * force creation of these "Action"s as needed by registering them to syntax that we expect to see
   * even
   * if those "Action"s  don't normally pick up parameters from the input file.
   */

  // clang-format off
  /**************************/
  /**** Register Actions ****/
  /**************************/
  registerMooseObjectTask("create_problem",               Problem,                 true);
  registerMooseObjectTask("setup_executioner",            Executioner,             true);

  // This task does not construct an object, but it needs all of the parameters that
  // would normally be used to construct an object.
  registerMooseObjectTask("determine_system_type",        Executioner,             true);

  registerMooseObjectTask("setup_mesh",                   MooseMesh,              false);
  registerMooseObjectTask("init_mesh",                    MooseMesh,              false);
  registerMooseObjectTask("add_mesh_modifier",            MeshModifier,           false);

  registerMooseObjectTask("add_kernel",                   Kernel,                 false);
  appendMooseObjectTask  ("add_kernel",                   EigenKernel);

  registerMooseObjectTask("add_nodal_kernel",             NodalKernel,            false);

  registerMooseObjectTask("add_material",                 Material,               false);
  registerMooseObjectTask("add_bc",                       BoundaryCondition,      false);
  registerMooseObjectTask("add_function",                 Function,               false);
  registerMooseObjectTask("add_distribution",             Distribution,           false);

  registerMooseObjectTask("add_aux_kernel",               AuxKernel,              false);
  registerMooseObjectTask("add_elemental_field_variable", AuxKernel,              false);

  registerMooseObjectTask("add_scalar_kernel",            ScalarKernel,           false);
  registerMooseObjectTask("add_aux_scalar_kernel",        AuxScalarKernel,        false);
  registerMooseObjectTask("add_dirac_kernel",             DiracKernel,            false);
  registerMooseObjectTask("add_dg_kernel",                DGKernel,               false);
  registerMooseObjectTask("add_interface_kernel",         InterfaceKernel,        false);
  registerMooseObjectTask("add_constraint",               Constraint,             false);

  registerMooseObjectTask("add_ic",                       InitialCondition,       false);
  appendMooseObjectTask  ("add_ic",                       ScalarInitialCondition);

  registerMooseObjectTask("add_damper",                   Damper,                 false);
  registerMooseObjectTask("setup_predictor",              Predictor,              false);
  registerMooseObjectTask("setup_time_stepper",           TimeStepper,            false);
  registerMooseObjectTask("setup_time_integrator",        TimeIntegrator,         false);

  registerMooseObjectTask("add_preconditioning",          MoosePreconditioner,    false);
  registerMooseObjectTask("add_field_split",              Split,                  false);

  registerMooseObjectTask("add_user_object",              UserObject,             false);
  appendMooseObjectTask  ("add_user_object",              Postprocessor);

  registerMooseObjectTask("add_postprocessor",            Postprocessor,          false);
  registerMooseObjectTask("add_vector_postprocessor",     VectorPostprocessor,    false);

  registerMooseObjectTask("add_indicator",                Indicator,              false);
  registerMooseObjectTask("add_marker",                   Marker,                 false);

  registerMooseObjectTask("add_multi_app",                MultiApp,               false);
  registerMooseObjectTask("add_transfer",                 Transfer,               false);

  registerMooseObjectTask("add_output",                   Output,                 false);

  registerMooseObjectTask("add_control",                  Control,                false);
  registerMooseObjectTask("add_partitioner",              MoosePartitioner,       false);

  // clang-format on

  registerTask("dynamic_object_registration", false);
  registerTask("common_output", true);
  registerTask("setup_recover_file_base", true);

  registerTask("add_bounds_vectors", false);
  registerTask("add_periodic_bc", false);
  registerTask("add_aux_variable", false);
  registerTask("add_variable", false);

  registerTask("execute_mesh_modifiers", false);
  registerTask("uniform_refine_mesh", false);
  registerTask("prepare_mesh", false);
  registerTask("setup_mesh_complete", false); // calls prepare

  registerTask("init_displaced_problem", false);

  registerTask("init_problem", true);
  registerTask("check_copy_nodal_vars", true);
  registerTask("copy_nodal_vars", true);
  registerTask("copy_nodal_aux_vars", true);
  registerTask("setup_postprocessor_data", false);

  registerTask("setup_dampers", true);
  registerTask("check_integrity", true);
  registerTask("setup_quadrature", true);

  /// Additional Actions
  registerTask("no_action", false); // Used for Empty Action placeholders
  registerTask("set_global_params", false);
  registerTask("setup_adaptivity", false);
  registerTask("meta_action", false);
  registerTask("setup_debug", false);
  registerTask("setup_residual_debug", false);
  registerTask("setup_oversampling", false);
  registerTask("deprecated_block", false);
  registerTask("set_adaptivity_options", false);
  registerTask("add_mortar_interface", false);

  // Dummy Actions (useful for sync points in the dependencies)
  registerTask("setup_function_complete", false);
  registerTask("setup_variable_complete", false);
  registerTask("ready_to_init", true);

  // Output related actions
  registerTask("setup_material_output", true);
  registerTask("check_output", true);

  /**************************/
  /****** Dependencies ******/
  /**************************/
  /**
   * The following is the default set of action dependencies for a basic MOOSE problem.  The
   * formatting
   * of this string is important.  Each line represents a set of dependencies that depend on the
   * previous
   * line.  Items on the same line have equal weight and can be executed in any order.
   *
   * Additional dependencies can be inserted later inside of user applications with calls to
   * ActionWarehouse::addDependency("task", "pre_req")
   */
  syntax.addDependencySets("(meta_action)"
                           "(dynamic_object_registration)"
                           "(common_output)"
                           "(set_global_params)"
                           "(setup_recover_file_base)"
                           "(check_copy_nodal_vars)"
                           "(setup_mesh)"
                           "(add_partitioner)"
                           "(init_mesh)"
                           "(prepare_mesh)"
                           "(add_mesh_modifier)"
                           "(execute_mesh_modifiers)"
                           "(add_mortar_interface)"
                           "(uniform_refine_mesh)"
                           "(setup_mesh_complete)"
                           "(determine_system_type)"
                           "(create_problem)"
                           "(setup_time_integrator)"
                           "(setup_executioner)"
                           "(setup_time_stepper)"
                           "(setup_predictor)"
                           "(setup_postprocessor_data)"
                           "(init_displaced_problem)"
                           "(add_aux_variable, add_variable, add_elemental_field_variable)"
                           "(setup_variable_complete)"
                           "(setup_quadrature)"
                           "(add_function)"
                           "(add_distribution)"
                           "(add_periodic_bc)"
                           "(add_user_object)"
                           "(setup_function_complete)"
                           "(setup_adaptivity)"
                           "(set_adaptivity_options)"
                           "(add_ic)"
                           "(add_constraint, add_field_split)"
                           "(add_preconditioning)"
                           "(ready_to_init)"
                           "(setup_dampers)"
                           "(setup_residual_debug)"
                           "(add_bounds_vectors)"
                           "(add_multi_app)"
                           "(add_transfer)"
                           "(copy_nodal_vars, copy_nodal_aux_vars)"
                           "(add_material)"
                           "(setup_material_output)"
                           "(init_problem)"
                           "(setup_debug)"
                           "(add_output)"
                           "(add_postprocessor)"
                           "(add_vector_postprocessor)" // MaterialVectorPostprocessor requires this
                                                        // to be after material objects are created.
                           "(add_aux_kernel, add_bc, add_damper, add_dirac_kernel, add_kernel, "
                           "add_nodal_kernel, add_dg_kernel, add_interface_kernel, "
                           "add_scalar_kernel, add_aux_scalar_kernel, add_indicator, add_marker)"
                           "(add_control)"
                           "(check_output)"
                           "(check_integrity)");
}

/**
 * Multiple Action class can be associated with a single input file section, in which case all
 * associated Actions
 * will be created and "acted" on when the associated input file section is seen.b *
 * Example:
 *  "setup_mesh" <-----------> SetupMeshAction <---------
 *                                                        \
 *                                                         [Mesh]
 *                                                        /
 * "setup_mesh_complete" <---> SetupMeshCompleteAction <-
 *
 *
 * Action classes can also be registered to act on more than one input file section for a different
 * task
 * if similar logic can work in multiple cases
 *
 * Example:
 * "add_variable" <-----                       -> [Variables/ *]
 *                       \                   /
 *                        CopyNodalVarsAction
 *                       /                   \
 * "add_aux_variable" <-                       -> [AuxVariables/ *]
 *
 *
 * Note: Placeholder "no_action" actions must be put in places where it is possible to match an
 * object
 *       with a star or a more specific parent later on. (i.e. where one needs to negate the '*'
 * matching
 *       prematurely)
 */
void
registerActions(Syntax & syntax, ActionFactory & action_factory)
{

#undef registerAction
#define registerAction(tplt, action)                                                               \
  action_factory.reg<tplt>(stringifyName(tplt), action, __FILE__, __LINE__)

  registerAction(SetupPostprocessorDataAction, "setup_postprocessor_data");

  registerAction(SetupMeshAction, "setup_mesh");
  registerAction(SetupMeshAction, "init_mesh");
  registerAction(SetupMeshCompleteAction, "prepare_mesh");
  registerAction(AddMeshModifierAction, "add_mesh_modifier");
  registerAction(AddMortarInterfaceAction, "add_mortar_interface");
  registerAction(SetupMeshCompleteAction, "execute_mesh_modifiers");
  registerAction(SetupMeshCompleteAction, "uniform_refine_mesh");
  registerAction(SetupMeshCompleteAction, "setup_mesh_complete");

  registerAction(AddFunctionAction, "add_function");
  registerAction(AddDistributionAction, "add_distribution");
  registerAction(CreateExecutionerAction, "setup_executioner");
  registerAction(SetupTimeStepperAction, "setup_time_stepper");
  registerAction(SetupTimeIntegratorAction, "setup_time_integrator");
  registerAction(CreateDisplacedProblemAction, "init_displaced_problem");
  registerAction(DetermineSystemType, "determine_system_type");
  registerAction(CreateProblemAction, "create_problem");
  registerAction(DynamicObjectRegistrationAction, "dynamic_object_registration");
  registerAction(AddOutputAction, "add_output");
  registerAction(CommonOutputAction, "common_output");
  registerAction(SetupRecoverFileBaseAction, "setup_recover_file_base");
  registerAction(GlobalParamsAction, "set_global_params");
  registerAction(SetupPredictorAction, "setup_predictor");
  registerAction(MaterialOutputAction, "setup_material_output");
  registerAction(CheckOutputAction, "check_output");

  /// Variable/AuxVariable Actions
  registerAction(AddVariableAction, "add_variable");
  registerAction(AddAuxVariableAction, "add_aux_variable");

  registerAction(CopyNodalVarsAction, "check_copy_nodal_vars");
  registerAction(CopyNodalVarsAction, "copy_nodal_vars");
  registerAction(CopyNodalVarsAction, "copy_nodal_aux_vars");

  // Initial Condition Actions
  registerAction(AddICAction, "add_ic");
  registerAction(AddInitialConditionAction, "add_ic");

  registerAction(AddKernelAction, "add_kernel");
  registerAction(AddNodalKernelAction, "add_nodal_kernel");
  registerAction(AddKernelAction, "add_aux_kernel");
  registerAction(AddScalarKernelAction, "add_scalar_kernel");
  registerAction(AddScalarKernelAction, "add_aux_scalar_kernel");
  registerAction(AddDGKernelAction, "add_dg_kernel");
  registerAction(AddInterfaceKernelAction, "add_interface_kernel");
  registerAction(AddBCAction, "add_bc");
  registerAction(EmptyAction, "no_action"); // placeholder
  registerAction(AddPeriodicBCAction, "add_periodic_bc");
  registerAction(AddMaterialAction, "add_material");
  registerAction(AddPostprocessorAction, "add_postprocessor");
  registerAction(AddVectorPostprocessorAction, "add_vector_postprocessor");
  registerAction(AddDamperAction, "add_damper");
  registerAction(AddFieldSplitAction, "add_field_split");
  registerAction(SetupPreconditionerAction, "add_preconditioning");
  registerAction(SetupQuadratureAction, "setup_quadrature");
  registerAction(DeprecatedBlockAction, "deprecated_block");
  registerAction(AddConstraintAction, "add_constraint");
  registerAction(AddUserObjectAction, "add_user_object");
  registerAction(AddControlAction, "add_control");
  registerAction(AddElementalFieldAction, "add_elemental_field_variable");
  registerAction(AddIndicatorAction, "add_indicator");
  registerAction(AddMarkerAction, "add_marker");
  registerAction(SetAdaptivityOptionsAction, "set_adaptivity_options");

  registerAction(AddNodalNormalsAction, "add_aux_variable");
  registerAction(AddNodalNormalsAction, "add_postprocessor");
  registerAction(AddNodalNormalsAction, "add_user_object");

#ifdef LIBMESH_ENABLE_AMR
  registerAction(AdaptivityAction, "setup_adaptivity");
#endif

  registerAction(PartitionerAction, "add_partitioner");
  registerAction(AddDiracKernelAction, "add_dirac_kernel");
  registerAction(SetupDebugAction, "setup_debug");
  registerAction(SetupResidualDebugAction, "setup_residual_debug");

  registerAction(AddBoundsVectorsAction, "add_bounds_vectors");

  // NonParsedActions
  registerAction(SetupDampersAction, "setup_dampers");
  registerAction(EmptyAction, "ready_to_init");
  registerAction(InitProblemAction, "init_problem");
  registerAction(CheckIntegrityAction, "check_integrity");

  registerAction(AddMultiAppAction, "add_multi_app");
  registerAction(AddTransferAction, "add_transfer");

  // TODO: Why is this here?
  registerTask("finish_input_file_output", false);
  registerAction(EmptyAction, "finish_input_file_output");

#undef registerAction
#define registerAction(tplt, action) action_factory.regLegacy<tplt>(stringifyName(tplt), action)
}

void
registerExecFlags()
{
  registerExecFlag(EXEC_NONE, "NONE");                     // 0
  registerExecFlag(EXEC_INITIAL, "INITIAL");               // 1
  registerExecFlag(EXEC_LINEAR, "LINEAR");                 // 2
  registerExecFlag(EXEC_NONLINEAR, "NONLINEAR");           // 4
  registerExecFlag(EXEC_TIMESTEP_END, "TIMESTEP_END");     // 8
  registerExecFlag(EXEC_TIMESTEP_BEGIN, "TIMESTEP_BEGIN"); // 16
  registerExecFlag(EXEC_FINAL, "FINAL");                   // 32
  registerExecFlag(EXEC_FORCED, "FORCED");                 // 64
  registerExecFlag(EXEC_FAILED, "FAILED");                 // 128
  registerExecFlag(EXEC_CUSTOM, "CUSTOM");                 // 256
  registerExecFlag(EXEC_SUBDOMAIN, "SUBDOMAIN");           // 512
  registerExecFlag(EXEC_SAME_AS_MULTIAPP, "SAME_AS_MULTIAPP");
}

void
setSolverDefaults(FEProblemBase & problem)
{
#ifdef LIBMESH_HAVE_PETSC
  // May be a touch expensive to create a new DM every time, but probably safer to do it this way
  Moose::PetscSupport::petscSetDefaults(problem);
#endif // LIBMESH_HAVE_PETSC
}

MPI_Comm
swapLibMeshComm(MPI_Comm new_comm)
{
#ifdef LIBMESH_HAVE_PETSC
  MPI_Comm old_comm = PETSC_COMM_WORLD;
  PETSC_COMM_WORLD = new_comm;
  return old_comm;
#endif // LIBMESH_HAVE_PETSC
}

void
enableFPE(bool on)
{
  if (_trap_fpe)
    libMesh::enableFPE(on);
}

PerfLog setup_perf_log("Setup");

/**
 * Initialize global variables
 */
#ifdef DEBUG
bool _trap_fpe = true;
#else
bool _trap_fpe = false;
#endif

static bool _color_console = isatty(fileno(stdout));

bool
colorConsole()
{
  return _color_console;
}

bool
setColorConsole(bool use_color, bool force)
{
  _color_console = (isatty(fileno(stdout)) || force) && use_color;
  return _color_console;
}

bool _warnings_are_errors = false;

bool _deprecated_is_error = false;

bool _throw_on_error = false;

std::map<ExecFlagType, std::string> execute_flags = std::map<ExecFlagType, std::string>();

} // namespace Moose

// This is outside of the MOOSE names space so that it works without Moose::, like the other
// registration macros.
void
registerExecFlag(const ExecFlagType & flag, const std::string & str)
{
  Moose::execute_flags[flag] = str;
}
