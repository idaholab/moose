#include "KernelFactory.h"
#include "BodyForce.h"
#include "Diffusion.h"
#include "Reaction.h"
#include "CoupledForce.h"

#include "BCFactory.h"
#include "DirichletBC.h"
#include "SinDirichletBC.h"
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
#include "MaterialsBlock.h"
#include "GenericMaterialBlock.h"
#include "ExecutionBlock.h"
#include "TransientBlock.h"
#include "OutputBlock.h"
#include "PreconditioningBlock.h"
#include "PBPBlock.h"
#include "AdaptivityBlock.h"
#include "GenericICBlock.h"

#include "ComputeInitialConditions.h"
#include "InitialConditionFactory.h"
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "RandomIC.h"

#include "Moose.h"
#include "PetscSupport.h"

#include "ParallelUniqueId.h"

//libMesh includes
#include "mesh.h"
#include "mesh_refinement.h"
#include "boundary_info.h"
#include "gmv_io.h"
#include "exodusII_io.h"
#include "tecplot_io.h"


void
Moose::registerObjects()
{
  static bool first = true;
  if(first)
  {
    first = false;
    ParallelUniqueId::initialize();

    Kernel::sizeEverything();
    BoundaryCondition::sizeEverything();
    AuxKernel::sizeEverything();
  }
  
  KernelFactory::instance()->registerKernel<BodyForce>("BodyForce");
  KernelFactory::instance()->registerKernel<Diffusion>("Diffusion");
  KernelFactory::instance()->registerKernel<Reaction>("Reaction");
  KernelFactory::instance()->registerKernel<ImplicitEuler>("ImplicitEuler");
  KernelFactory::instance()->registerKernel<ImplicitBackwardDifference2>("ImplicitBackwardDifference2");
  KernelFactory::instance()->registerKernel<CoupledForce>("CoupledForce");
  
  BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
  BCFactory::instance()->registerBC<SinDirichletBC>("SinDirichletBC");
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
  ParserBlockFactory::instance()->registerParserBlock<MaterialsBlock>("Materials");
  ParserBlockFactory::instance()->registerParserBlock<GenericMaterialBlock>("Materials/*");
  ParserBlockFactory::instance()->registerParserBlock<ExecutionBlock>("Execution");
  ParserBlockFactory::instance()->registerParserBlock<TransientBlock>("Execution/Transient");
  ParserBlockFactory::instance()->registerParserBlock<AdaptivityBlock>("Execution/Adaptivity");
  ParserBlockFactory::instance()->registerParserBlock<OutputBlock>("Output");
  ParserBlockFactory::instance()->registerParserBlock<PreconditioningBlock>("Preconditioning");
  ParserBlockFactory::instance()->registerParserBlock<PBPBlock>("Preconditioning/PBP");

  InitialConditionFactory::instance()->registerInitialCondition<ConstantIC>("ConstantIC");  
  InitialConditionFactory::instance()->registerInitialCondition<BoundingBoxIC>("BoundingBoxIC");  
  InitialConditionFactory::instance()->registerInitialCondition<RandomIC>("RandomIC");  
}

void
Moose::meshChanged()
{
  // Reinitialize the equation_systems object for the newly refined
  // mesh. One of the steps in this is project the solution onto the 
  // new mesh
  Moose::equation_system->reinit();

  // Rebuild the boundary conditions 
  Moose::mesh->boundary_info->build_node_list_from_side_list();

  // Rebuild the active local element range
  delete Moose::active_local_elem_range;
  Moose::active_local_elem_range = NULL;

  // Calling this function will rebuild the range.
  Moose::getActiveLocalElementRange();

  // Lets the output system know that the mesh has changed recently.
  Moose::mesh_changed = true;
}

ConstElemRange *
Moose::getActiveLocalElementRange()
{
  if(!Moose::active_local_elem_range)
  {
    Moose::active_local_elem_range = new ConstElemRange(Moose::mesh->active_local_elements_begin(),
                                                        Moose::mesh->active_local_elements_end(),1);
  }

  return Moose::active_local_elem_range;  
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

/**
 * Outputs the system.
 */
void
Moose::output_system(EquationSystems * equation_systems, std::string file_base, unsigned int t_step, Real time, bool exodus_output, bool gmv_output, bool tecplot_output, bool print_out_info)
{
  OStringStream stream_file_base;
  
  stream_file_base << file_base << "_";
  OSSRealzeroright(stream_file_base,3,0,t_step);

  std::string file_name = stream_file_base.str();

  if(print_out_info)
     std::cout << "   --> Output in file ";
  
  if(exodus_output) 
  {
    std::string exodus_file_name;
    
    static ExodusII_IO * ex_out = NULL;
    static unsigned int num_files = 0;
    static unsigned int num_in_current_file = 0;
    
    bool adaptivity = Moose::equation_system->parameters.have_parameter<bool>("adaptivity");

    //if the mesh changed we need to write to a new file
    if(Moose::mesh_changed || !ex_out)
    {
      num_files++;
      
      if(ex_out)
        delete ex_out;
      
      ex_out = new ExodusII_IO(equation_systems->get_mesh());

      // We've captured this change... let's reset the changed bool and then see if it's changed again next time.
      Moose::mesh_changed = false;

      // We're starting over
      num_in_current_file = 0;
    }

    num_in_current_file++;

    if(!adaptivity)
      exodus_file_name = file_base;
    else
    {
      OStringStream exodus_stream_file_base;
  
      exodus_stream_file_base << file_base << "_";

      // -1 is so that the first one that comes out is 000
      OSSRealzeroright(exodus_stream_file_base,4,0,num_files-1);
      
      exodus_file_name = exodus_stream_file_base.str();
    }

    // The +1 is because Exodus starts timesteps at 1 and we start at 0
    ex_out->write_timestep(exodus_file_name + ".e", *equation_systems, num_in_current_file, time);

    if(print_out_info)
    {       
      std::cout << file_base+".e";
      OStringStream out;
      out <<  "(";
      OSSInt(out,2,t_step+1);
      out <<  ") ";
      std::cout << out.str();
    } 
  }
  if(gmv_output) 
  {     
    GMVIO(*Moose::mesh).write_equation_systems(file_name + ".gmv", *equation_systems);
    if(print_out_info)
    {       
      if(exodus_output)
         std::cout << " and ";    
      std::cout << file_name+".gmv";
    }    
  }  
  if(tecplot_output) 
  {     
    TecplotIO(*Moose::mesh).write_equation_systems(file_name + ".plt", *equation_systems);
    if(print_out_info)
    {       
      if(exodus_output || gmv_output)
         std::cout << " and ";    
      std::cout << file_name+".plt";
    }    
  }

  if(print_out_info) std::cout << std::endl; 
}

/******************
 * Global Variables
 * ****************/
THREAD_ID Moose::current_thread_id = 0;

Mesh * Moose::mesh;
ExodusII_IO * Moose::exreader;

EquationSystems * Moose::equation_system;

MeshRefinement * Moose::mesh_refinement = NULL;
ErrorEstimator * Moose::error_estimator = NULL;
ErrorVector * Moose::error = NULL;
bool Moose::mesh_changed = false;

ConstElemRange * Moose::active_local_elem_range = NULL;

enum Moose::GeomType;
Moose::GeomType Moose::geom_type = Moose::XYZ;

bool Moose::no_fe_reinit = false;

std::string Moose::execution_type;

std::string Moose::file_base = "";
int Moose::interval = 1;
bool Moose::exodus_output = false;
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

