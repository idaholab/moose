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

#include "Moose.h"

#include "ParallelUniqueId.h"

//libMesh includes
#include "mesh.h"
#include "boundary_info.h"


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
  ParserBlockFactory::instance()->registerParserBlock<OutputBlock>("Output");
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

/******************
 * Global Variables
 * ****************/
THREAD_ID Moose::current_thread_id = 0;

Mesh * Moose::mesh;
ExodusII_IO * Moose::exreader;
EquationSystems * Moose::equation_system;

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
MeshRefinement * Moose::mesh_refinement = NULL;
std::vector<Real> Moose::manual_scaling;



