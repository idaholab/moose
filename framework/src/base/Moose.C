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

#include "ComputeResidual.h"
#include "ComputeJacobian.h"
#include "MooseFactory.h"

#include "ParsedFunction.h"
#include "ParsedGradFunction.h"
#include "PiecewiseLinear.h"

#include "BodyForce.h"
#include "Diffusion.h"
//#include "Convection.h"
//#include "ConvectionWithDG.h"
#include "Reaction.h"
#include "CoupledForce.h"
#include "RealPropertyOutput.h"
#include "UserForcingFunction.h"

#include "DGDiffusion.h"
#include "EnhancedDGMatDiffusion.h"
//#include "DGConvection.h"

#include "DirichletBC.h"
#include "DirichletPostprocessorBC.h"
#include "SinDirichletBC.h"
#include "SinNeumannBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"
#include "VacuumBC.h"
#include "MatchedValueBC.h"
#include "ConvectiveFluxBC.h"
#include "WeakGradientBC.h"
#include "FunctionDirichletBC.h"
#include "FunctionNeumannBC.h"
#include "DGFunctionDiffusionDirichletBC.h"
#include "DashpotBC.h"
#include "ContactSlave.h"

#include "ConstantAux.h"
#include "CoupledAux.h"
#include "PenetrationAux.h"
#include "NearestNodeDistanceAux.h"
#include "NearestNodeValueAux.h"
#include "FunctionAux.h"
#include "ProcessorIDAux.h"

#include "EmptyFunction.h"

#include "TimeDerivative.h"
#include "ImplicitEuler.h"
#include "ImplicitBackwardDifference2.h"

#include "EmptyMaterial.h"
#include "GenericConstantMaterial.h"

#include "MeshBlock.h"
#include "MeshGenerationBlock.h"
#include "exodusII_io.h"
#include "VariablesBlock.h"
#include "GenericVariableBlock.h"
#include "AuxVariablesBlock.h"
#include "KernelsBlock.h"
#include "DGKernelsBlock.h"
#include "FunctionsBlock.h"
#include "GenericFunctionsBlock.h"
#include "GenericKernelBlock.h"
#include "GenericDGKernelBlock.h"
#include "AuxKernelsBlock.h"
#include "GenericAuxKernelBlock.h"
#include "BCsBlock.h"
#include "GenericBCBlock.h"
#include "StabilizersBlock.h"
#include "GenericStabilizerBlock.h"
#include "MaterialsBlock.h"
#include "GenericMaterialBlock.h"
#include "OutputBlock.h"
#include "PreconditioningBlock.h"
#include "PBPBlock.h"
#include "AdaptivityBlock.h"
#include "GenericICBlock.h"
#include "PeriodicBlock.h"
#include "GenericPeriodicBlock.h"
#include "GenericExecutionerBlock.h"
#include "PostprocessorsBlock.h"
#include "GenericPostprocessorBlock.h"
#include "GlobalParamsBlock.h"

#include "ComputeInitialConditions.h"
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "RandomIC.h"
//#include "FunctionIC.h"

#include "Steady.h"
#include "TransientExecutioner.h"
#include "SolutionTimeAdaptive.h"
#include "ExactSolutionExecutioner.h"
#include "DT2Transient.h"

#include "ConvectionDiffusionSUPG.h"

#include "ElementIntegral.h"
#include "ElementL2Error.h"
#include "ElementH1Error.h"
#include "ElementH1SemiError.h"
#include "ElementAverageValue.h"
#include "SideIntegral.h"
#include "SideAverageValue.h"
#include "PrintDOFs.h"
#include "PrintNumElems.h"
#include "PrintNumNodes.h"
#include "PrintDT.h"
#include "AverageElementSize.h"
#include "EmptyPostprocessor.h"
#include "SideFluxIntegral.h"
#include "NodalVariableValue.h"
#include "AverageNodalVariableValue.h"
#include "Reporter.h"

#include "Damper.h"
#include "DampersBlock.h"
#include "GenericDamperBlock.h"
#include "ConstantDamper.h"
#include "MaxIncrement.h"

#include "DiracKernel.h"
#include "DiracKernelsBlock.h"
#include "GenericDiracKernelBlock.h"
#include "ConstantPointSource.h"

#include "Moose.h"
#include "PetscSupport.h"

#include "ParallelUniqueId.h"

//libMesh includes
#include "mesh.h"
#include "boundary_info.h"
#include "parallel.h"

#include <set>

MooseInit::MooseInit(int argc, char** argv)
  :LibMeshInit(argc, argv)
{
  Moose::command_line = new GetPot(argc, argv);

  std::cout << "Number of Threads: " << libMesh::n_threads() << "\n";

  // Ignore remaining command line options
  PetscOptionsSetValue("-options_left", "no");
  
  ParallelUniqueId::initialize();
}

MooseInit::~MooseInit()
{
  delete Moose::command_line;
}

void
Moose::registerObjects()
{
  registerFunction(EmptyFunction);
  registerFunction(ParsedFunction);
  registerFunction(ParsedGradFunction);
  registerFunction(PiecewiseLinear);

  registerKernel(BodyForce);
  registerKernel(Diffusion);
  registerKernel(Reaction);
  registerKernel(TimeDerivative);
  registerKernel(ImplicitEuler);
  registerKernel(ImplicitBackwardDifference2);
  registerKernel(CoupledForce);
  registerKernel(RealPropertyOutput);
  registerKernel(UserForcingFunction);
//  registerKernel(Convection);
//  registerKernel(ConvectionWithDG);

  registerDGKernel(DGDiffusion);
  registerDGKernel(EnhancedDGMatDiffusion);
//  registerDGKernel(DGConvection);

  registerBoundaryCondition(DirichletBC);
  registerBoundaryCondition(DirichletPostprocessorBC);
  registerBoundaryCondition(SinDirichletBC);
  registerBoundaryCondition(SinNeumannBC);
  registerBoundaryCondition(NeumannBC);
  registerBoundaryCondition(VectorNeumannBC);
  registerBoundaryCondition(VacuumBC);
  registerBoundaryCondition(MatchedValueBC);
  registerBoundaryCondition(ConvectiveFluxBC);
  registerBoundaryCondition(WeakGradientBC);
  registerBoundaryCondition(FunctionDirichletBC);
  registerBoundaryCondition(FunctionNeumannBC);
  registerBoundaryCondition(DGFunctionDiffusionDirichletBC);
  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(ContactSlave);

  registerAux(ConstantAux);
  registerAux(CoupledAux);
  registerAux(PenetrationAux);
  registerAux(NearestNodeDistanceAux);
  registerAux(NearestNodeValueAux);
  registerAux(FunctionAux);
  registerAux(ProcessorIDAux);
  

  registerMaterial(EmptyMaterial);
  registerMaterial(GenericConstantMaterial);

  registerNamedParserBlock(MeshBlock, "Mesh");
  registerNamedParserBlock(MeshGenerationBlock, "Mesh/Generation");
  registerNamedParserBlock(FunctionsBlock, "Functions");
  registerNamedParserBlock(GenericFunctionsBlock, "Functions/*");
  registerNamedParserBlock(VariablesBlock, "Variables");
  registerNamedParserBlock(GenericVariableBlock, "Variables/*");
  registerNamedParserBlock(GenericICBlock, "Variables/*/InitialCondition");
  registerNamedParserBlock(AuxVariablesBlock, "AuxVariables");
  registerNamedParserBlock(GenericICBlock, "AuxVariables/*/InitialCondition");
  // Reuse the GenericVariableBlock for AuxVariables/*
  registerNamedParserBlock(GenericVariableBlock, "AuxVariables/*");
  registerNamedParserBlock(KernelsBlock, "Kernels");
  registerNamedParserBlock(GenericKernelBlock, "Kernels/*");
  registerNamedParserBlock(DGKernelsBlock, "DGKernels");
  registerNamedParserBlock(GenericDGKernelBlock, "DGKernels/*");
  registerNamedParserBlock(AuxKernelsBlock, "AuxKernels");
  registerNamedParserBlock(GenericAuxKernelBlock, "AuxKernels/*");
  registerNamedParserBlock(BCsBlock, "BCs");
  registerNamedParserBlock(GenericBCBlock, "BCs/*");
  // Reuse the BCsBlock for AuxBCs
  registerNamedParserBlock(BCsBlock, "AuxBCs");
  // Reuse the GenericBCBlock for AuxBCs/*
  registerNamedParserBlock(GenericBCBlock, "AuxBCs/*");
  registerNamedParserBlock(StabilizersBlock, "Stabilizers");
  registerNamedParserBlock(GenericStabilizerBlock, "Stabilizers/*");
  registerNamedParserBlock(MaterialsBlock, "Materials");
  registerNamedParserBlock(GenericMaterialBlock, "Materials/*");
  registerNamedParserBlock(OutputBlock, "Output");
  registerNamedParserBlock(PreconditioningBlock, "Preconditioning");
  registerNamedParserBlock(PBPBlock, "Preconditioning/PBP");
  registerNamedParserBlock(PeriodicBlock, "BCs/Periodic");
  registerNamedParserBlock(GenericPeriodicBlock, "BCs/Periodic/*");
  registerNamedParserBlock(GenericExecutionerBlock, "Executioner");
  registerNamedParserBlock(AdaptivityBlock, "Executioner/Adaptivity");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/*");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors/Residual");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/Residual/*");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors/Jacobian");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/Jacobian/*");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors/NewtonIter");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/Jacobian/*");
  registerNamedParserBlock(DampersBlock, "Dampers");
  registerNamedParserBlock(GenericDamperBlock, "Dampers/*");  
  registerNamedParserBlock(GlobalParamsBlock, "GlobalParams");
  registerNamedParserBlock(DiracKernelsBlock, "DiracKernels");
  registerNamedParserBlock(GenericDiracKernelBlock, "DiracKernels/*");  
  
  
  registerInitialCondition(ConstantIC);  
  registerInitialCondition(BoundingBoxIC);  
  registerInitialCondition(RandomIC);

  registerExecutioner(Steady);

  // Just in this one case to avoid a collision with libMesh am I registering something with a different
  // name than the class name!
  registerNamedExecutioner(TransientExecutioner, "Transient");
  registerExecutioner(SolutionTimeAdaptive);
  registerExecutioner(ExactSolutionExecutioner);
  registerExecutioner(DT2Transient);
  
  registerStabilizer(ConvectionDiffusionSUPG);

  registerPostprocessor(ElementIntegral);
  registerPostprocessor(ElementL2Error);
  registerPostprocessor(ElementH1Error);
  registerPostprocessor(ElementH1SemiError);
  registerPostprocessor(ElementAverageValue);

  registerPostprocessor(SideIntegral);
  registerPostprocessor(SideAverageValue);

  registerPostprocessor(PrintDOFs);
  registerPostprocessor(PrintNumElems);
  registerPostprocessor(PrintNumNodes);
  registerPostprocessor(PrintDT);
  registerPostprocessor(AverageElementSize);
  registerPostprocessor(EmptyPostprocessor);
  registerPostprocessor(SideFluxIntegral);
  registerPostprocessor(NodalVariableValue);
  registerPostprocessor(AverageNodalVariableValue);
  registerPostprocessor(Reporter);

  registerDamper(ConstantDamper);
  registerDamper(MaxIncrement);

  registerDiracKernel(ConstantPointSource);
}

void
Moose::setSolverDefaults(MooseSystem &moose_system, Executioner *executioner)
{
#ifdef LIBMESH_HAVE_PETSC
  MoosePetscSupport::petscSetDefaults(moose_system, executioner);
#endif //LIBMESH_HAVE_PETSC
}


/******************
 * Global Variables
 * ****************/

// This variable will be static in the new Moose System object - only need one per application
GetPot *Moose::command_line;
