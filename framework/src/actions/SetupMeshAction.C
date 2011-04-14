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

#include "SetupMeshAction.h"
#include "Parser.h"
#include "MooseMesh.h"
#include "MProblem.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<SetupMeshAction>()
{
  InputParameters params = validParams<Action>();

  // FIXME: Make this work (requires double registration in ActionFactory)
//  params.addParam<bool>("second_order", false, "Turns on second order elements for the input mesh");
//  params.addParam<std::string>("partitioner", "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation");
//  params.addParam<int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");
  
  return params;
}

SetupMeshAction::SetupMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}


void
SetupMeshAction::setupMesh(MooseMesh *mesh)
{
  // FIXME: second order
  //  if (getParam<bool>("second_order"))
  //    mesh->all_second_order(true);
	
  // FIXME: usage of partitioners
  //  if (getParam<std::string>("partitioner") == "linear")
  //    mesh->partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);
  
  Moose::setup_perf_log.push("Prepare Mesh","Setup");
  mesh->prepare();
  Moose::setup_perf_log.pop("Prepare Mesh","Setup");

#ifdef LIBMESH_ENABLE_AMR
  Moose::setup_perf_log.push("Uniformly Refine Mesh","Setup");
  // uniformly refine mesh
  mesh->uniformlyRefine();
  Moose::setup_perf_log.pop("Uniformly Refine Mesh","Setup");
#endif //LIBMESH_ENABLE_AMR

  // FIXME: autosize problem
  //  MeshRefinement mesh_refinement(*mesh);
  //  if (!autoResizeProblem(mesh, mesh_refinement))
  //    mesh_refinement.uniformly_refine(getParam<int>("uniform_refine"));
    
  Moose::setup_perf_log.push("Initial meshChanged()","Setup");
  mesh->meshChanged();
  Moose::setup_perf_log.pop("Initial meshChanged()","Setup");
    
  mesh->printInfo();
}

void
SetupMeshAction::act()
{
  if (_parser_handle._mesh)
    setupMesh(_parser_handle._mesh);
  else
    mooseError("No mesh file was supplied and no generation block was provided");

  if (_parser_handle._displaced_mesh)
    setupMesh(_parser_handle._displaced_mesh);
  
  // There is no setup execution action satisfied, create the MProblem class by ourselves
  if (Moose::action_warehouse.actionBlocksWithActionBegin("setup_executioner") ==
      Moose::action_warehouse.actionBlocksWithActionEnd("setup_executioner"))
  {
    Moose::setup_perf_log.push("Create MProblem","Setup");
    _parser_handle._problem = new MProblem(*_parser_handle._mesh);
    Moose::setup_perf_log.pop("Create MProblem","Setup");
  }
}

