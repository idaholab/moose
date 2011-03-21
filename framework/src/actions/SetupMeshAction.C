#include "SetupMeshAction.h"
#include "Parser.h"
#include "MooseMesh.h"
#include "MProblem.h"
#include "ActionWarehouse.h"

// libMesh includes
#include "exodusII_io.h"

const std::string SetupMeshAction::no_file_supplied("(no file supplied)");

template<>
InputParameters validParams<SetupMeshAction>()
{
  InputParameters params = validParams<Action>();
//  params.addParam<int>("dim", "DEPRECATED - Mesh dim can be determined from the file read.");
  params.addParam<std::string>("file", SetupMeshAction::no_file_supplied, "The name of the mesh file to read (required unless using dynamic generation)");
  params.addParam<bool>("second_order", false, "Turns on second order elements for the input mesh");
  params.addParam<std::string>("partitioner", "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation");
  params.addParam<int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");
  params.addParam<std::vector<std::string> >("displacements", "The variables corresponding to the x y z displacements of the mesh.  If this is provided then the displacements will be taken into account during the computation.");
  return params;
}

SetupMeshAction::SetupMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
   std::cerr << "Constructing SetupMeshAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
SetupMeshAction::act()
{
  std::cerr << "Acting on SetupMeshAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";


//  int mesh_dim = isParamValid("dim") ? getParam<int>("dim") : 1;

  /**
   * We will use the mesh object to read the file to cut down on
   * I/O conntention.  We still need to use the Exodus reader though
   * for copy_nodal_solutions
   */

  
  //if (ParserBlock *gen_block = locateBlock("Mesh/Generation"))
  //  gen_block->execute();
  
  if (getParam<std::string>("file") != no_file_supplied)
  {
    mooseAssert(_parser_handle._mesh == NULL, "Mesh already exists, and you are trying to read another");
    _parser_handle._mesh = new MooseMesh();
    _parser_handle._exreader = new ExodusII_IO(*_parser_handle._mesh);
    _parser_handle._exreader->read(getParam<std::string>("file"));
  }
  // get convenience pointer to mesh object
  MooseMesh *mesh = _parser_handle._mesh;

  if (mesh != NULL)
  {

  // FIXME: second order
//  if (getParam<bool>("second_order"))
//    mesh->all_second_order(true);

  // FIXME: usage of partitioners
//  if (getParam<std::string>("partitioner") == "linear")
//    mesh->partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);

  mesh->prepare();

  // uniformly refine mesh
  mesh->uniformlyRefine(getParam<int>("uniform_refine"));

  // FIXME: autosize problem
//  MeshRefinement mesh_refinement(*mesh);
//  if (!autoResizeProblem(mesh, mesh_refinement))
//    mesh_refinement.uniformly_refine(getParam<int>("uniform_refine"));

  mesh->meshChanged();

//  mesh->print_info();
  }

  if (isParamValid("displacements"))
  {
    std::vector<std::string> displacements = getParam<std::vector<std::string> >("displacements");
    if (displacements.size() != _parser_handle._mesh->dimension())
      mooseError("Number of displacements and dimension of mesh MUST be the same!");
  }

  // There is no setup execution action satisfied, create the MProblem class by ourselves
  if (Moose::action_warehouse.actionBlocksWithActionBegin("setup_executioner") ==
      Moose::action_warehouse.actionBlocksWithActionEnd())
    _parser_handle._problem = new MProblem(*mesh);
}

