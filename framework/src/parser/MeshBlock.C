#include "GenericVariableBlock.h"

// Moose Includes
#include "MeshBlock.h"
#include "Moose.h"

// libMesh includes
#include "mesh.h"
#include "exodusII_io.h"
#include "boundary_info.h"
#include "getpot.h"
#include "mesh_refinement.h"
#include "linear_partitioner.h"

MeshBlock::MeshBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  addParam<int>("dim", -1, "The dimension of the mesh file to read or generate", true);
  addParam<std::string>("file", "", "The name of the mesh file to read (required unless using dynamic generation)", false);
  addParam<bool>("second_order", false, "Turns on second order elements for the input mesh", false);
  addParam<bool>("generated", false, "Tell MOOSE that a mesh will be generated", false);
  addParam<std::string>("partitioner", "", "Specifies a mesh partitioner to use when spliting the mesh for a parallel computation", false);
  addParam<int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh", false);
}

void
MeshBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MeshBlock Object\n";
#endif

  Mesh *mesh = new Mesh(getParamValue<int>("dim"));
  Moose::mesh = mesh;

  // TODO: Need test for Mesh Generation
  if (getParamValue<bool>("generated")) 
  {
    if (ParserBlock *gen_block = locateBlock("Mesh/Generation"))
      gen_block->execute();
    else
      mooseError();
  }
  
  if (detectRestart()) 
  {
    ExodusII_IO *exreader = new ExodusII_IO(*mesh);
    Moose::exreader = exreader;
    exreader->read(getParamValue<std::string>("file"));
  }
  else
    /* We will use the mesh object to read the file to cut down on
     * I/O conntention.  We still need to use the Exodus reader though
     *for copy_nodal_solutions
     */
    mesh->read(getParamValue<std::string>("file"));

  if (getParamValue<bool>("second_order"))
    mesh->all_second_order(true);

  if (getParamValue<std::string>("partitioner") == "linear")
    mesh->partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);
  mesh->prepare_for_use(false);

  // If using ParallelMesh this will delete non-local elements from the current processor
  mesh->delete_remote_elements();

  MeshRefinement *mesh_refinement = new MeshRefinement(*mesh);
  mesh_refinement->uniformly_refine(getParamValue<int>("uniform_refine"));
  Moose::mesh_refinement = mesh_refinement;
  
  mesh->boundary_info->build_node_list_from_side_list();
  mesh->print_info();

  visitChildren();
}

bool
MeshBlock::detectRestart()
{
  std::vector<std::string> blocks_to_check(2);
  blocks_to_check[0] = "Variables";
  blocks_to_check[1] = "AuxVariables";
  
  for (int i=0; i<blocks_to_check.size(); ++i) 
  {
    ParserBlock *vars_block = locateBlock(blocks_to_check[i]);
    if (vars_block != NULL)
      for (PBChildIterator j = vars_block->_children.begin(); j != vars_block->_children.end(); ++j) 
      {
        GenericVariableBlock *v = dynamic_cast<GenericVariableBlock *>(*j);
        if (v && v->restartRequired())
          return true;
      }
  }
  return false;
}
