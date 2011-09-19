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

#include "ReadMeshAction.h"
#include "Parser.h"
#include "MooseMesh.h"
#include "MProblem.h"
#include "ActionWarehouse.h"

// libMesh includes
#include "exodusII_io.h"
#include "nemesis_io.h"
#include "parallel_mesh.h"

const std::string ReadMeshAction::no_file_supplied("(no file supplied)");

template<>
InputParameters validParams<ReadMeshAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("file", ReadMeshAction::no_file_supplied, "The name of the mesh file to read (required unless using dynamic generation)");
  params.addParam<std::vector<std::string> >("displacements", "The variables corresponding to the x y z displacements of the mesh.  If this is provided then the displacements will be taken into account during the computation.");
  params.addParam<bool>("nemesis", false, "If nemesis=true and file=foo.e, actually reads foo.e.N.0, foo.e.N.1, ... foo.e.N.N-1, where N = # CPUs, with NemesisIO.");
  params.addParam<std::vector<unsigned int> >("ghosted_boundaries", "Boundaries to be ghosted if using Nemesis");
  params.addParam<std::vector<Real> >("ghosted_boundaries_inflation", "If you are using ghosted boundaries you will want to set this value to a vector of amounts to inflate the bounding boxes by.  ie if you are running a 3D problem you might set it to '0.2 0.1 0.4'");
  params.addParam<bool>("skip_partitioning", false, "If true the mesh won't be partitioned.  Probably not a good idea to use it with a serial mesh!");
  params.addParam<unsigned int>("patch_size", 40, "The number of nodes to consider in the NearestNode neighborhood.");

  return params;
}

ReadMeshAction::ReadMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
ReadMeshAction::act()
{
  std::string mesh_file = getParam<std::string>("file");
  if (mesh_file != no_file_supplied)
  {
    mooseAssert(_parser_handle._mesh == NULL, "Mesh already exists, and you are trying to read another");

    // Create the mesh and save it off
    _parser_handle._mesh = new MooseMesh();
    _parser_handle._mesh->setPatchSize(getParam<unsigned int>("patch_size"));

    Parser::checkFileReadable(mesh_file);

    Moose::setup_perf_log.push("Read Mesh","Setup");
    if (getParam<bool>("nemesis"))
    {
      // Nemesis_IO only takes a reference to ParallelMesh, so we can't be quite so short here.
      ParallelMesh& pmesh = libmesh_cast_ref<ParallelMesh&>(_parser_handle._mesh->getMesh());
      Nemesis_IO(pmesh).read(mesh_file);
      //_parser_handle._mesh->parallel(true); // This is redundant because we have Mesh::is_serial()
    }
    else // not reading Nemesis files
    {
      // if reading ExodusII, read it through a reader and save it off, since it will be used in possible "copy nodal vars" action
      // NOTE: the other reader that can do copy nodal values is GMVIO, but GMV is _pretty_ old right now (May 2011)
      if (mesh_file.rfind(".exd") < mesh_file.size() ||
          mesh_file.rfind(".e") < mesh_file.size())
      {
        _parser_handle._exreader = new ExodusII_IO(*_parser_handle._mesh);
        _parser_handle._exreader->read(mesh_file);
      }
      else
        _parser_handle._mesh->read(mesh_file);
    }
    Moose::setup_perf_log.pop("Read Mesh","Setup");

    _parser_handle._mesh->_mesh.skip_partitioning(getParam<bool>("skip_partitioning"));

    if (isParamValid("displacements"))
    {
      // Create the displaced mesh
      _parser_handle._displaced_mesh = new MooseMesh();
      _parser_handle._displaced_mesh->setPatchSize(getParam<unsigned int>("patch_size"));

      Moose::setup_perf_log.push("Read Displaced Mesh","Setup");

      if (getParam<bool>("nemesis"))
      {
        // Nemesis_IO only takes a reference to ParallelMesh
        ParallelMesh& pmesh = libmesh_cast_ref<ParallelMesh&>(_parser_handle._displaced_mesh->getMesh());
        Nemesis_IO(pmesh).read(mesh_file);
      }
      else // not reading Nemesis files
      {
        // Here we are fine with read, since we are not doing "copy_nodal_vars" on displaced mesh (yet ;-))
        _parser_handle._displaced_mesh->read(mesh_file);
      }

      Moose::setup_perf_log.pop("Read Displaced Mesh","Setup");

      std::vector<std::string> displacements = getParam<std::vector<std::string> >("displacements");
      if (displacements.size() != _parser_handle._mesh->dimension())
        mooseError("Number of displacements and dimension of mesh MUST be the same!");

      _parser_handle._displaced_mesh->_mesh.skip_partitioning(getParam<bool>("skip_partitioning"));
    }
  }

  std::vector<unsigned int> ghosted_boundaries = getParam<std::vector<unsigned int > >("ghosted_boundaries");

  for(unsigned int i=0; i<ghosted_boundaries.size(); i++)
  {
    _parser_handle._mesh->addGhostedBoundary(ghosted_boundaries[i]);
    if (isParamValid("displacements"))
      _parser_handle._displaced_mesh->addGhostedBoundary(ghosted_boundaries[i]);
  }

  if(isParamValid("ghosted_boundaries_inflation"))
  {
    std::vector<Real> ghosted_boundaries_inflation = getParam<std::vector<Real> >("ghosted_boundaries_inflation");
    _parser_handle._mesh->setGhostedBoundaryInflation(ghosted_boundaries_inflation);
    if (isParamValid("displacements"))
      _parser_handle._displaced_mesh->setGhostedBoundaryInflation(ghosted_boundaries_inflation);
  }

  mooseAssert(_parser_handle._mesh != NULL, "Mesh hasn't been created");
}
