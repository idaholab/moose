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
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "ProblemFactory.h"

// libmesh includes
#include "linear_partitioner.h"
#include "centroid_partitioner.h"

template<>
InputParameters validParams<SetupMeshAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<bool>("second_order", false, "Turns on second order elements for the input mesh");
  params.addParam<std::string>("partitioner", "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation. Available options: linear, centroid");
  params.addParam<std::string>("centroid_partitioner_direction", "Specifies the sort direction if using the centroid partitioner. Available options: x, y, z, radial");
  params.addParam<bool>("construct_side_list_from_node_list", false, "If true, construct side lists from the nodesets in the mesh (i.e. if every node on a give side is in a nodeset then add that side to a sideset");

  params.addParam<std::vector<SubdomainID> >("block_id", "IDs of the block id/name pairs");
  params.addParam<std::vector<SubdomainName> >("block_name", "Names of the block id/name pairs (must correspond with \"block_id\"");

  params.addParam<std::vector<BoundaryID> >("boundary_id", "IDs of the boundary id/name pairs");
  params.addParam<std::vector<BoundaryName> >("boundary_name", "Names of the boundary id/name pairs (must correspond with \"boundary_id\"");
  return params;
}

SetupMeshAction::SetupMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupMeshAction::setupMesh(MooseMesh *mesh)
{
  if (getParam<bool>("second_order"))
    mesh->_mesh.all_second_order(true);

  if (getParam<std::string>("partitioner") == "linear")
    mesh->_mesh.partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);

  if (getParam<std::string>("partitioner") == "centroid")
  {
    if(!isParamValid("centroid_partitioner_direction"))
      mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

    std::string direction = getParam<std::string>("centroid_partitioner_direction");

    std::cout<<"Direction: "<<direction;

    if(direction == "x")
      mesh->_mesh.partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::X));
    else if(direction == "y")
      mesh->_mesh.partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::Y));
    else if(direction == "z")
      mesh->_mesh.partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::Z));
    else if(direction == "radial")
      mesh->_mesh.partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::RADIAL));
    else
      mooseError("Invalid centroid_partitioner_direction!");
  }

  Moose::setup_perf_log.push("Prepare Mesh","Setup");
  mesh->prepare();
  Moose::setup_perf_log.pop("Prepare Mesh","Setup");

  if (getParam<bool>("construct_side_list_from_node_list"))
    mesh->_mesh.boundary_info->build_side_list_from_node_list();

  Moose::setup_perf_log.push("Initial meshChanged()","Setup");
  mesh->meshChanged();
  Moose::setup_perf_log.pop("Initial meshChanged()","Setup");

  // Add names to the mesh
  if (_pars.isParamValid("block_id") && _pars.isParamValid("block_name"))
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID> >("block_id");
    std::vector<SubdomainName> names = getParam<std::vector<SubdomainName> >("block_name");
    std::set<SubdomainName> seen_it;

    if (ids.size() != names.size())
      mooseError("You must supply the same number of block ids and names parameters");

    for (unsigned int i=0; i<ids.size(); ++i)
    {
      if (seen_it.find(names[i]) != seen_it.end())
        mooseError("The following dynamic block name is not unique: " + names[i]);
      seen_it.insert(names[i]);
      mesh->subdomainName(ids[i]) = names[i];
    }

  }
  if (_pars.isParamValid("boundary_id") && _pars.isParamValid("boundary_name"))
  {
    std::vector<BoundaryID> ids = getParam<std::vector<BoundaryID> >("boundary_id");
    std::vector<BoundaryName> names = getParam<std::vector<BoundaryName> >("boundary_name");
    std::set<SubdomainName> seen_it;

    if (ids.size() != names.size())
      mooseError("You must supply the same number of boundary ids and names parameters");

    for (unsigned int i=0; i<ids.size(); ++i)
    {
      if (seen_it.find(names[i]) != seen_it.end())
        mooseError("The following dynamic boundary name is not unique: " + names[i]);
      mesh->boundaryName(ids[i]) = names[i];
      seen_it.insert(names[i]);
    }
  }

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
    Moose::setup_perf_log.push("Create FEProblem","Setup");

    // Use the Factory to build a normal MOOSE problem
    _parser_handle._problem = ProblemFactory::instance()->createFEProblem(_parser_handle._mesh);
    Moose::setup_perf_log.pop("Create FEProblem","Setup");
  }
}
