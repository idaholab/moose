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

#include "MeshExtruder.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/boundary_info.h"

template<>
InputParameters validParams<MeshExtruder>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file to read");
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers in the extruded mesh");
  params.addRequiredParam<RealVectorValue>("extrusion_vector", "The direction and length of the extrusion");
  params.addParam<boundary_id_type>("bottom_sideset", "The boundary id that will be applied to the bottom of the extruded mesh");
  params.addParam<boundary_id_type>("top_sideset", "The boundary id that will be to the top of the extruded mesh");
  return params;
}

MeshExtruder::MeshExtruder(const std::string & name, InputParameters parameters):
    MooseMesh(name, parameters),
    _num_layers(getParam<unsigned int>("num_layers")),
    _extrusion_vector(getParam<RealVectorValue>("extrusion_vector"))
{
}

MeshExtruder::MeshExtruder(const MeshExtruder & other_mesh) :
    MooseMesh(other_mesh),
    _num_layers(other_mesh._num_layers),
    _extrusion_vector(other_mesh._extrusion_vector)
{
}

MeshExtruder::~MeshExtruder()
{
}

MooseMesh &
MeshExtruder::clone() const
{
  return *(new MeshExtruder(*this));
}

void
MeshExtruder::init()
{
  libMesh::Mesh src_mesh;

  // Read in the 2D Mesh
  src_mesh.read(getParam<MeshFileName>("file"));

  // Let libMesh do the heavy lifting ;)
  MeshTools::Generation::build_extrusion(_mesh, src_mesh, _num_layers, _extrusion_vector);

  // See if the user has requested specific sides for the top and bottom
  const std::set<boundary_id_type> &side_ids =
    _mesh.boundary_info->get_side_boundary_ids();
  std::set<boundary_id_type>::reverse_iterator last_side_it = side_ids.rbegin();

  const boundary_id_type old_top = *last_side_it;
  mooseAssert(last_side_it != side_ids.rend(), "Error in generating sidesets for extruded mesh");
  const boundary_id_type old_bottom = *++last_side_it;

  if (isParamValid("bottom_sideset"))
  {
    boundary_id_type bottom_sideset = getParam<boundary_id_type>("bottom_sideset");
    if (bottom_sideset != old_bottom)
      changeBoundaryId(old_bottom, bottom_sideset, true);
  }
  if (isParamValid("top_sideset"))
  {
    boundary_id_type top_sideset = getParam<boundary_id_type>("top_sideset");
    if (top_sideset != old_top)
      changeBoundaryId(old_top, top_sideset, true);
  }
}
