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
#include "MooseApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "Factory.h"

// libmesh includes
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "MooseEnum.h"

template<>
InputParameters validParams<SetupMeshAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.set<std::string>("type") = "FileMesh";

  params.addParam<bool>("second_order", false, "Converts a first order mesh to a second order mesh.  Note: This is NOT needed if you are reading an actual first order mesh.");

  MooseEnum partitioning("metis, linear, centroid, parmetis", "metis");
  params.addParam<MooseEnum>("partitioner", partitioning, "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");

  MooseEnum direction("x, y, z, radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction", direction, "Specifies the sort direction if using the centroid partitioner. Available options: x, y, z, radial");

  params.addParam<std::vector<SubdomainID> >("block_id", "IDs of the block id/name pairs");
  params.addParam<std::vector<SubdomainName> >("block_name", "Names of the block id/name pairs (must correspond with \"block_id\"");

  params.addParam<std::vector<BoundaryID> >("boundary_id", "IDs of the boundary id/name pairs");
  params.addParam<std::vector<BoundaryName> >("boundary_name", "Names of the boundary id/name pairs (must correspond with \"boundary_id\"");

  params.addParam<bool>("construct_side_list_from_node_list", false, "If true, construct side lists from the nodesets in the mesh (i.e. if every node on a give side is in a nodeset then add that side to a sideset");

  params.addParam<std::vector<std::string> >("displacements", "The variables corresponding to the x y z displacements of the mesh.  If this is provided then the displacements will be taken into account during the computation.");
  params.addParam<std::vector<BoundaryName> >("ghosted_boundaries", "Boundaries to be ghosted if using Nemesis");
  params.addParam<std::vector<Real> >("ghosted_boundaries_inflation", "If you are using ghosted boundaries you will want to set this value to a vector of amounts to inflate the bounding boxes by.  ie if you are running a 3D problem you might set it to '0.2 0.1 0.4'");
  params.addParam<unsigned int>("patch_size", 40, "The number of nodes to consider in the NearestNode neighborhood.");
  params.addParam<unsigned int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");

  // groups
  params.addParamNamesToGroup("displacements ghosted_boundaries ghosted_boundaries_inflation patch_size", "Advanced");
  params.addParamNamesToGroup("second_order construct_side_list_from_node_list", "Advanced");
  params.addParamNamesToGroup("partitioner centroid_partitioner_direction", "Partitioning");
  params.addParamNamesToGroup("block_id block_name boundary_id boundary_name", "Add Names");

  return params;
}

SetupMeshAction::SetupMeshAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
SetupMeshAction::setupMesh(MooseMesh *mesh)
{
  std::vector<BoundaryName> ghosted_boundaries = getParam<std::vector<BoundaryName> >("ghosted_boundaries");
  for(unsigned int i=0; i<ghosted_boundaries.size(); i++)
    mesh->addGhostedBoundary(mesh->getBoundaryID(ghosted_boundaries[i]));

  mesh->setPatchSize(getParam<unsigned int>("patch_size"));

  if(isParamValid("ghosted_boundaries_inflation"))
  {
    std::vector<Real> ghosted_boundaries_inflation = getParam<std::vector<Real> >("ghosted_boundaries_inflation");
    mesh->setGhostedBoundaryInflation(ghosted_boundaries_inflation);
  }

  mesh->ghostGhostedBoundaries();

  if (getParam<bool>("second_order"))
    mesh->getMesh().all_second_order(true);

  // Note: Metis is the default partitioner so we don't even test for that case
  if (getParam<MooseEnum>("partitioner") == "parmetis")
    mesh->getMesh().partitioner() = AutoPtr<Partitioner>(new ParmetisPartitioner);
  else if (getParam<MooseEnum>("partitioner") == "linear")
    mesh->getMesh().partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);
  else if (getParam<MooseEnum>("partitioner") == "centroid")
  {
    if(!isParamValid("centroid_partitioner_direction"))
      mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

    MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

    if(direction == "x")
      mesh->getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::X));
    else if(direction == "y")
      mesh->getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::Y));
    else if(direction == "z")
      mesh->getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::Z));
    else if(direction == "radial")
      mesh->getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::RADIAL));
    else
      mooseError("Invalid centroid_partitioner_direction!");
  }

  if (getParam<MooseEnum>("partitioner") != "metis") // NOT the default
    mesh->getMesh().prepare_for_use(); // repartition

#ifdef LIBMESH_ENABLE_AMR
  unsigned int level = getParam<unsigned int>("uniform_refine");

  // Did they specify extra refinement levels on the command-line?
  level += _app.getParam<unsigned int>("refinements");

  mesh->uniformRefineLevel() = level;
#endif //LIBMESH_ENABLE_AMR

  // Add entity names to the mesh
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
      mesh->setSubdomainName(ids[i], names[i]);
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
      mesh->setBoundaryName(ids[i], names[i]);
      seen_it.insert(names[i]);
    }
  }

  if (getParam<bool>("construct_side_list_from_node_list"))
    mesh->getMesh().boundary_info->build_side_list_from_node_list();

}

void
SetupMeshAction::act()
{
  if (_type == "MooseMesh")
  {
    mooseDeprecated();
    Moose::err << "Warning: MooseMesh is gone - please use FileMesh instead!";
    _type = "FileMesh";
  }

  // Create the mesh object and tell it to build itself
  _mesh = dynamic_cast<MooseMesh *>(_factory.create(_type, "mesh", _moose_object_pars));
  _mesh->init();

  mooseAssert(_mesh != NULL, "Mesh hasn't been created");

  if (isParamValid("displacements"))
  {
    // Create the displaced mesh
    _displaced_mesh = dynamic_cast<MooseMesh *>(_factory.create(_type, "displaced_mesh", _moose_object_pars));
    _displaced_mesh->init();

    std::vector<std::string> displacements = getParam<std::vector<std::string> >("displacements");
    if (displacements.size() != _displaced_mesh->dimension())
      mooseError("Number of displacements and dimension of mesh MUST be the same!");
  }

  setupMesh(_mesh);

  if (_displaced_mesh)
    setupMesh(_displaced_mesh);
}
