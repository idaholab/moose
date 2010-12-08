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

#include "GenericVariableBlock.h"

// Moose Includes
#include "MeshBlock.h"
#include "Moose.h"
#include "Parser.h"

// libMesh includes
#include "mesh.h"
#include "exodusII_io.h"
#include "boundary_info.h"
#include "getpot.h"
#include "mesh_refinement.h"
#include "linear_partitioner.h"
#include "mesh_tools.h"

template<>
InputParameters validParams<MeshBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<int>("dim", "DEPRECATED - Mesh dim can be determined from the file read.");
  params.addParam<std::string>("file", "(no file supplied)", "The name of the mesh file to read (required unless using dynamic generation)");
  params.addParam<bool>("second_order", false, "Turns on second order elements for the input mesh");
  params.addParam<std::string>("partitioner", "Specifies a mesh partitioner to use when spliting the mesh for a parallel computation");
  params.addParam<int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");
  params.addParam<std::vector<std::string> >("displacements", "The variables corresponding to the x y z displacements of the mesh.  If this is provided then the displacements will be taken into account during the computation.");

  return params;
}

MeshBlock::MeshBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params)
{}

void
MeshBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the MeshBlock Object\n";
#endif

  int mesh_dim = isParamValid("dim") ? getParamValue<int>("dim") : 1;
  Mesh *mesh = _moose_system.initMesh(mesh_dim);

  /**
   * We will use the mesh object to read the file to cut down on
   * I/O conntention.  We still need to use the Exodus reader though
   * for copy_nodal_solutions
   */
  
  if (ParserBlock *gen_block = locateBlock("Mesh/Generation"))
    gen_block->execute();
  else if (checkVariableProperties(&GenericVariableBlock::restartRequired)) 
    _moose_system.getExodusReader()->read(getParamValue<std::string>("file"));
  else
    mesh->read(getParamValue<std::string>("file"));

  // Tell MooseSystem that the dimension of the mesh has changed
  _moose_system.updateDimension();
  
  if (getParamValue<bool>("second_order"))
    mesh->all_second_order(true);

  if (getParamValue<std::string>("partitioner") == "linear")
    mesh->partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);
  mesh->prepare_for_use(false);

  // If using ParallelMesh this will delete non-local elements from the current processor
  mesh->delete_remote_elements();

  // uniformly refine mesh
  MeshRefinement *mesh_refinement = _moose_system.initMeshRefinement();
  if (!autoResizeProblem(mesh, *mesh_refinement))
    mesh_refinement->uniformly_refine(getParamValue<int>("uniform_refine"));
    
  //  _moose_system.meshChanged();
  mesh->boundary_info->build_node_list_from_side_list();
  MeshTools::build_nodes_to_elem_map(*mesh, _moose_system.node_to_elem_map);

  mesh->print_info();


  if(isParamValid("displacements"))
  {
    std::vector<std::string> displacements = getParamValue<std::vector<std::string> >("displacements");

    if(displacements.size() != mesh->mesh_dimension())
      mooseError("Number of displacements and dimension of mesh MUST be the same!");

    Mesh * displaced_mesh = _moose_system.initDisplacedMesh(displacements);

    displaced_mesh->prepare_for_use(false);

    (*displaced_mesh->boundary_info) = (*mesh->boundary_info);

    displaced_mesh->boundary_info->build_node_list_from_side_list();

    _moose_system.needSerializedSolution(true);
  }

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
