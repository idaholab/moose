// Moose Includes
#include "MeshBlock.h"
#include "Moose.h"

// libMesh includes
#include "mesh.h"
#include "exodusII_io.h"
#include "boundary_info.h"
#include "getpot.h"

MeshBlock::MeshBlock(const std::string & reg_id, const std::string & real_id, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, input_file)
{
  _block_params.set<int>("dim") = 3;
  _block_params.set<std::string>("file");
}

void
MeshBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MeshBlock Object\n";
#endif

  Mesh *mesh = new Mesh(_block_params.get<int>("dim"));
  Moose::mesh = mesh;

  // TODO: Make this generic to use the libMesh reader when possible
  // TODO: Might need to save handle to ExodusII reader
  ExodusII_IO exreader(*mesh);
  exreader.read(_block_params.get<std::string>("file"));
  mesh->prepare_for_use(false);

  // TODO: Check for Mesh Refinement Params
  // TODO: Check for Mesh Partitioner Params
  // TODO: Check for Mesh Restart Params inside of VariablesBlock
  
  mesh->boundary_info->build_node_list_from_side_list();
  mesh->print_info();
}
