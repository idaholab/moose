// Moose Includes
#include "MeshBlock.h"
#include "Parser.h"
#include "GenericVariableBlock.h"
#include "GenericExecutionerBlock.h"
#include "Init.h"
#include "MProblem.h"

// libMesh includes
#include "mesh.h"
#include "exodusII_io.h"
#include "boundary_info.h"
#include "getpot.h"
#include "mesh_refinement.h"
#include "mesh_generation.h"
#include "linear_partitioner.h"
#include "mesh_tools.h"


const std::string no_file_supplied("(no file supplied)");

template<>
InputParameters validParams<MeshBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<int>("dim", "DEPRECATED - Mesh dim can be determined from the file read.");
  params.addParam<std::string>("file", no_file_supplied, "The name of the mesh file to read (required unless using dynamic generation)");
  params.addParam<bool>("second_order", false, "Turns on second order elements for the input mesh");
  params.addParam<std::string>("partitioner", "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation");
  params.addParam<int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");
  params.addParam<std::vector<std::string> >("displacements", "The variables corresponding to the x y z displacements of the mesh.  If this is provided then the displacements will be taken into account during the computation.");

  return params;
}


MeshBlock::MeshBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params)
{
}

void
MeshBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MeshBlock Object\n";
#endif

  int mesh_dim = isParamValid("dim") ? getParamValue<int>("dim") : 1;

  /**
   * We will use the mesh object to read the file to cut down on
   * I/O conntention.  We still need to use the Exodus reader though
   * for copy_nodal_solutions
   */

  if (ParserBlock *gen_block = locateBlock("Mesh/Generation"))
    gen_block->execute();
  else if (getParamValue<std::string>("file") != no_file_supplied)
  {
    _parser_handle._mesh = new Moose::Mesh(mesh_dim);
    _parser_handle._exreader = new ExodusII_IO(*_parser_handle._mesh);
    _parser_handle._exreader->read(getParamValue<std::string>("file"));
  }
  // get convenience pointer to mesh object
  Moose::Mesh *mesh = _parser_handle._mesh;

  if (mesh != NULL)
  {

  // FIXME: second order
//  if (getParamValue<bool>("second_order"))
//    mesh->all_second_order(true);

  // FIXME: usage of partitioners
//  if (getParamValue<std::string>("partitioner") == "linear")
//    mesh->partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);

  mesh->prepare();

  // uniformly refine mesh
  mesh->uniformlyRefine(getParamValue<int>("uniform_refine"));

  // FIXME: autosize problem
//  MeshRefinement mesh_refinement(*mesh);
//  if (!autoResizeProblem(mesh, mesh_refinement))
//    mesh_refinement.uniformly_refine(getParamValue<int>("uniform_refine"));

  mesh->meshChanged();

//  mesh->print_info();
  }

  if (isParamValid("displacements"))
  {
    std::vector<std::string> displacements = getParamValue<std::vector<std::string> >("displacements");
    if (displacements.size() != _parser_handle._mesh->dimension())
      mooseError("Number of displacements and dimension of mesh MUST be the same!");
  }

  // There is no executioner block, create the MProblem class by ourselves
  GenericExecutionerBlock * exec = dynamic_cast<GenericExecutionerBlock *>(locateBlock("Executioner"));
  if (exec == NULL)
    _parser_handle._problem = new Moose::MProblem(*mesh);

  visitChildren();
}

bool
MeshBlock::autoResizeProblem(Mesh *mesh, MeshRefinement &mesh_refinement)
{
  unsigned int requested_dofs;

  // See if dofs was passed on the command line
  if (Moose::command_line != NULL && Moose::command_line->search("--dofs"))
    requested_dofs = Moose::command_line->next(-1);
  else
    return false;

  // See if the variables report themselves as resizable
  if (!checkVariableProperties(&GenericVariableBlock::autoResizeable))
    return false;

  unsigned int num_vars = locateBlock("Variables")->n_activeChildren();
  while (mesh->n_nodes() * num_vars < requested_dofs)
    mesh_refinement.uniformly_refine();
  return true;
}

bool
MeshBlock::checkVariableProperties(bool (GenericVariableBlock::*property)() const)
{
  std::vector<std::string> blocks_to_check(2);
  blocks_to_check[0] = "Variables";
  blocks_to_check[1] = "AuxVariables";
  
  for (unsigned int i=0; i<blocks_to_check.size(); ++i) 
  {
    ParserBlock *vars_block = locateBlock(blocks_to_check[i]);
    if (vars_block != NULL)
      for (PBChildIterator j = vars_block->_children.begin(); j != vars_block->_children.end(); ++j) 
      {
        GenericVariableBlock *v = dynamic_cast<GenericVariableBlock *>(*j);
        if (v && (v->*property)())
          return true;
      }
  }
  return false;
}

