#include "GenericVariableBlock.h"

// Moose Includes
#include "MeshBlock.h"
#include "Moose.h"

// libMesh includes
#include "mesh.h"
#include "exodusII_io.h"
#include "boundary_info.h"
#include "getpot.h"

MeshBlock::MeshBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<int>("dim") = 3;
  _block_params.set<std::string>("file");
  _block_params.set<bool>("second_order") = false;
  _block_params.set<bool>("generated") = false;
}

void
MeshBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MeshBlock Object\n";
#endif

  Mesh *mesh = new Mesh(_block_params.get<int>("dim"));
  Moose::mesh = mesh;

  // TODO: Might need to save handle to ExodusII reader for copying the nodal solutions later
  // TODO: Copy Nodal Solutions
  if (detectRestart()) 
  {
    ExodusII_IO exreader(*mesh);
    exreader.read(_block_params.get<std::string>("file"));
  }
  else
    /* We will use the mesh object to read the file to cut down on
     * I/O conntention.  We still need to use the Exodus reader though
     *for copy_nodal_solutions
     */
    mesh->read(_block_params.get<std::string>("file"));

  // TODO: Fix this call - always breaks ???
  // mesh->all_second_order(_block_params.get<bool>("second_order"));

  mesh->prepare_for_use(false);

  // TODO: Need to make a Mesh Generation Block handler
  
  // TODO: Check for Mesh Refinement Params
  // TODO: Check for Mesh Partitioner Params
  
  mesh->boundary_info->build_node_list_from_side_list();
  mesh->print_info();
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

