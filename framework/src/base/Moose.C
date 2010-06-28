#include "KernelFactory.h"
#include "BodyForce.h"
#include "Diffusion.h"
#include "Reaction.h"
#include "CoupledForce.h"
#include "RealPropertyOutput.h"
#include "ForcingFunction.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "SinDirichletBC.h"
#include "SinNeumannBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"
#include "VacuumBC.h"
#include "MatchedValueBC.h"
#include "ConvectiveFluxBC.h"
#include "WeakGradientBC.h"

#include "AuxFactory.h"
#include "ConstantAux.h"
#include "CoupledAux.h"

#include "ImplicitEuler.h"
#include "ImplicitBackwardDifference2.h"

#include "MaterialFactory.h"
#include "EmptyMaterial.h"

#include "ParserBlockFactory.h"
#include "MeshBlock.h"
#include "MeshGenerationBlock.h"
#include "exodusII_io.h"
#include "VariablesBlock.h"
#include "GenericVariableBlock.h"
#include "AuxVariablesBlock.h"
#include "KernelsBlock.h"
#include "GenericKernelBlock.h"
#include "AuxKernelsBlock.h"
#include "GenericAuxKernelBlock.h"
#include "BCsBlock.h"
#include "GenericBCBlock.h"
#include "StabilizersBlock.h"
#include "GenericStabilizerBlock.h"
#include "MaterialsBlock.h"
#include "GenericMaterialBlock.h"
//#include "ExecutionBlock.h"
//#include "TransientBlock.h"
#include "OutputBlock.h"
#include "PreconditioningBlock.h"
#include "PBPBlock.h"
#include "AdaptivityBlock.h"
#include "GenericICBlock.h"
#include "PeriodicBlock.h"
#include "GenericPeriodicBlock.h"
#include "GenericExecutionerBlock.h"

#include "ComputeInitialConditions.h"
#include "InitialConditionFactory.h"
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "RandomIC.h"

#include "ExecutionerFactory.h"
#include "Steady.h"
#include "TransientExecutioner.h"
#include "SolutionTimeAdaptive.h"

#include "StabilizerFactory.h"
#include "ConvectionDiffusionSUPG.h"

#include "Moose.h"
#include "PetscSupport.h"

#include "ParallelUniqueId.h"

//libMesh includes
#include "mesh.h"
#include "mesh_refinement.h"
#include "boundary_info.h"
#include "parallel.h"

#include <set>

MooseInit::MooseInit(int argc, char** argv)
  :LibMeshInit(argc, argv)
{
  Moose::command_line = new GetPot(argc, argv);

  std::cout << "Number of Threads: " << libMesh::n_threads() << "\n";

  ParallelUniqueId::initialize();
}

MooseInit::~MooseInit()
{
  delete Moose::command_line;
}

void
Moose::registerObjects()
{
  static bool first = true;
  if(first)
  {
    first = false;
    ParallelUniqueId::initialize();
  }
  
  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  KernelFactory::instance()->registerKernel<Diffusion>("Diffusion");
  KernelFactory::instance()->registerKernel<Reaction>("Reaction");
  KernelFactory::instance()->registerKernel<ImplicitEuler>("ImplicitEuler");
  KernelFactory::instance()->registerKernel<ImplicitBackwardDifference2>("ImplicitBackwardDifference2");
  KernelFactory::instance()->registerKernel<CoupledForce>("CoupledForce");
  KernelFactory::instance()->registerKernel<RealPropertyOutput>("RealPropertyOutput");
  KernelFactory::instance()->registerKernel<ForcingFunction>("ForcingFunction");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<SinDirichletBC>("SinDirichletBC");
  BCFactory::instance()->registerBC<SinNeumannBC>("SinNeumannBC");
  BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
  BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");
  BCFactory::instance()->registerBC<VacuumBC>("VacuumBC");
  BCFactory::instance()->registerBC<MatchedValueBC>("MatchedValueBC");
  BCFactory::instance()->registerBC<ConvectiveFluxBC>("ConvectiveFluxBC");
  BCFactory::instance()->registerBC<WeakGradientBC>("WeakGradientBC");

  AuxFactory::instance()->registerAux<ConstantAux>("ConstantAux");
  AuxFactory::instance()->registerAux<CoupledAux>("CoupledAux");

  MaterialFactory::instance()->registerMaterial<EmptyMaterial>("EmptyMaterial");

  ParserBlockFactory::instance()->registerParserBlock<MeshBlock>("Mesh");
  ParserBlockFactory::instance()->registerParserBlock<MeshGenerationBlock>("Mesh/Generation");
  ParserBlockFactory::instance()->registerParserBlock<VariablesBlock>("Variables");
  ParserBlockFactory::instance()->registerParserBlock<GenericVariableBlock>("Variables/*");
  ParserBlockFactory::instance()->registerParserBlock<GenericICBlock>("Variables/*/InitialCondition");
  ParserBlockFactory::instance()->registerParserBlock<AuxVariablesBlock>("AuxVariables");
  ParserBlockFactory::instance()->registerParserBlock<GenericICBlock>("AuxVariables/*/InitialCondition");
  // Reuse the GenericVariableBlock for AuxVariables/*
  ParserBlockFactory::instance()->registerParserBlock<GenericVariableBlock>("AuxVariables/*");
  ParserBlockFactory::instance()->registerParserBlock<KernelsBlock>("Kernels");
  ParserBlockFactory::instance()->registerParserBlock<GenericKernelBlock>("Kernels/*");
  ParserBlockFactory::instance()->registerParserBlock<AuxKernelsBlock>("AuxKernels");
  ParserBlockFactory::instance()->registerParserBlock<GenericAuxKernelBlock>("AuxKernels/*");
  ParserBlockFactory::instance()->registerParserBlock<BCsBlock>("BCs");
  ParserBlockFactory::instance()->registerParserBlock<GenericBCBlock>("BCs/*");
  // Reuse the BCsBlock for AuxBCs
  ParserBlockFactory::instance()->registerParserBlock<BCsBlock>("AuxBCs");
  // Reuse the GenericBCBlock for AuxBCs/*
  ParserBlockFactory::instance()->registerParserBlock<GenericBCBlock>("AuxBCs/*");
  ParserBlockFactory::instance()->registerParserBlock<StabilizersBlock>("Stabilizers");
  ParserBlockFactory::instance()->registerParserBlock<GenericStabilizerBlock>("Stabilizers/*");
  ParserBlockFactory::instance()->registerParserBlock<MaterialsBlock>("Materials");
  ParserBlockFactory::instance()->registerParserBlock<GenericMaterialBlock>("Materials/*");
//  ParserBlockFactory::instance()->registerParserBlock<ExecutionBlock>("Execution");
//  ParserBlockFactory::instance()->registerParserBlock<TransientBlock>("Execution/Transient");
//  ParserBlockFactory::instance()->registerParserBlock<AdaptivityBlock>("Execution/Adaptivity");
  ParserBlockFactory::instance()->registerParserBlock<OutputBlock>("Output");
  ParserBlockFactory::instance()->registerParserBlock<PreconditioningBlock>("Preconditioning");
  ParserBlockFactory::instance()->registerParserBlock<PBPBlock>("Preconditioning/PBP");
  ParserBlockFactory::instance()->registerParserBlock<PeriodicBlock>("BCs/Periodic");
  ParserBlockFactory::instance()->registerParserBlock<GenericPeriodicBlock>("BCs/Periodic/*");
  ParserBlockFactory::instance()->registerParserBlock<GenericExecutionerBlock>("Executioner");
  ParserBlockFactory::instance()->registerParserBlock<AdaptivityBlock>("Executioner/Adaptivity");

  InitialConditionFactory::instance()->registerInitialCondition<ConstantIC>("ConstantIC");  
  InitialConditionFactory::instance()->registerInitialCondition<BoundingBoxIC>("BoundingBoxIC");  
  InitialConditionFactory::instance()->registerInitialCondition<RandomIC>("RandomIC");

  ExecutionerFactory::instance()->registerExecutioner<Steady>("Steady");

  // Just in this one case to avoid a collision with libMesh am I registering something with a different
  // name than the class name!
  ExecutionerFactory::instance()->registerExecutioner<TransientExecutioner>("Transient");
  ExecutionerFactory::instance()->registerExecutioner<SolutionTimeAdaptive>("SolutionTimeAdaptive");
  
  StabilizerFactory::instance()->registerStabilizer<ConvectionDiffusionSUPG>("ConvectionDiffusionSUPG");
}

void
Moose::setSolverDefaults(EquationSystems * es,
                         TransientNonlinearImplicitSystem & system,
                         void (*compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar),
                         void (*compute_residual) (const NumericVector<Number>& soln, NumericVector<Number>& residual))
{
#ifdef LIBMESH_HAVE_PETSC
  MoosePetscSupport::petscSetDefaults(es, system, compute_jacobian_block, compute_residual);
#endif //LIBMESH_HAVE_PETSC
}


/******************
 * Global Variables
 * ****************/
THREAD_ID Moose::current_thread_id = 0;

// FIXME
MooseSystem * Moose::g_system = NULL;

//EquationSystems * Moose::equation_system;

MeshRefinement * Moose::mesh_refinement = NULL;
ErrorEstimator * Moose::error_estimator = NULL;
ErrorVector * Moose::error = NULL;

Executioner * Moose::executioner;

enum Moose::GeomType;
Moose::GeomType Moose::geom_type = Moose::XYZ;

bool Moose::no_fe_reinit = false;

std::string Moose::execution_type;

std::string Moose::file_base = "out";
int Moose::interval = 1;
bool Moose::exodus_output = true;         // default output format is exodus
bool Moose::gmv_output = false;
bool Moose::tecplot_output = false;
bool Moose::print_out_info = false;
bool Moose::output_initial = false;
bool Moose::auto_scaling = false;
std::vector<Real> Moose::manual_scaling;
Number (*Moose::init_value)(const Point& p,
                            const Parameters& parameters,
                            const std::string& sys_name,
                            const std::string& var_name) = Moose::initial_value;
Gradient (*Moose::init_gradient)(const Point& p,
                                 const Parameters& parameters,
                                 const std::string& sys_name,
                                 const std::string& var_name) = Moose::initial_gradient;
void (*Moose::init_cond)(EquationSystems& es, const std::string& system_name) = Moose::initial_condition;

// This variable will be static in the new Moose System object - only need one per application
GetPot *Moose::command_line;

