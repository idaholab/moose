//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneratedMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/elem.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseApp", GeneratedMeshGenerator);

InputParameters
GeneratedMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum elem_types(LIST_GEOM_ELEM); // no default

  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  params.addParam<unsigned int>("nx", 1, "Number of elements in the X direction");
  params.addParam<unsigned int>("ny", 1, "Number of elements in the Y direction");
  params.addParam<unsigned int>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "The type of element from libMesh to "
                             "generate (default: linear element for "
                             "requested dimension)");
  params.addParam<std::vector<SubdomainID>>(
      "subdomain_ids",
      "Subdomain IDs for each element, default to all zero. If a single number is specified, that "
      "subdomain id is used for all element.");
  params.addParam<SubdomainName>("subdomain_name",
                                 "If specified, single subdomain name for all elements");

  params.addParam<bool>(
      "gauss_lobatto_grid",
      false,
      "Grade mesh into boundaries according to Gauss-Lobatto quadrature spacing.");
  params.addRangeCheckedParam<Real>(
      "bias_x",
      1.,
      "bias_x>=0.5 & bias_x<=2",
      "The amount by which to grow (or shrink) the cells in the x-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_y",
      1.,
      "bias_y>=0.5 & bias_y<=2",
      "The amount by which to grow (or shrink) the cells in the y-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_z",
      1.,
      "bias_z>=0.5 & bias_z<=2",
      "The amount by which to grow (or shrink) the cells in the z-direction.");

  params.addParam<std::string>("boundary_name_prefix",
                               "If provided, prefix the built in boundary names with this string");
  params.addParam<boundary_id_type>(
      "boundary_id_offset", 0, "This offset is added to the generated boundary IDs");

  params.addParam<std::vector<ExtraElementIDName>>("extra_element_integers",
                                                   "Names of extra element integers");

  params.addClassDescription(
      "Create a line, square, or cube mesh with uniformly spaced or biased elements.");

  return params;
}

GeneratedMeshGenerator::GeneratedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(declareMeshProperty("num_elements_x", getParam<unsigned int>("nx"))),
    _ny(declareMeshProperty("num_elements_y", getParam<unsigned int>("ny"))),
    _nz(declareMeshProperty("num_elements_z", getParam<unsigned int>("nz"))),
    _xmin(declareMeshProperty("xmin", getParam<Real>("xmin"))),
    _xmax(declareMeshProperty("xmax", getParam<Real>("xmax"))),
    _ymin(declareMeshProperty("ymin", getParam<Real>("ymin"))),
    _ymax(declareMeshProperty("ymax", getParam<Real>("ymax"))),
    _zmin(declareMeshProperty("zmin", getParam<Real>("zmin"))),
    _zmax(declareMeshProperty("zmax", getParam<Real>("zmax"))),
    _has_subdomain_ids(isParamValid("subdomain_ids")),
    _gauss_lobatto_grid(getParam<bool>("gauss_lobatto_grid")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z")),
    _boundary_name_prefix(isParamValid("boundary_name_prefix")
                              ? getParam<std::string>("boundary_name_prefix") + "_"
                              : ""),
    _boundary_id_offset(getParam<boundary_id_type>("boundary_id_offset"))
{
  if (_gauss_lobatto_grid && (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0))
    mooseError("Cannot apply both Gauss-Lobatto mesh grading and biasing at the same time.");
  if (_xmax < _xmin)
    paramError("xmax", "xmax must be larger than xmin.");
  if (_ymax < _ymin)
    paramError("ymax", "ymax must be larger than ymin.");
  if (_zmax < _zmin)
    paramError("zmax", "zmax must be larger than zmin.");
}

std::unique_ptr<MeshBase>
GeneratedMeshGenerator::generate()
{
  // Have MOOSE construct the correct libMesh::Mesh object using Mesh block and CLI parameters.
  auto mesh = buildMeshBaseObject();

  if (isParamValid("extra_element_integers"))
  {
    for (auto & name : getParam<std::vector<ExtraElementIDName>>("extra_element_integers"))
      mesh->add_elem_integer(name);
  }

  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
      case 1:
        elem_type_enum = "EDGE2";
        break;
      case 2:
        elem_type_enum = "QUAD4";
        break;
      case 3:
        elem_type_enum = "HEX8";
        break;
    }
  }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  // Switching on MooseEnum
  switch (_dim)
  {
    // The build_XYZ mesh generation functions take an
    // UnstructuredMesh& as the first argument, hence the static_cast.
    case 1:
      MeshTools::Generation::build_line(static_cast<UnstructuredMesh &>(*mesh),
                                        _nx,
                                        _xmin,
                                        _xmax,
                                        elem_type,
                                        _gauss_lobatto_grid);
      break;
    case 2:
      MeshTools::Generation::build_square(static_cast<UnstructuredMesh &>(*mesh),
                                          _nx,
                                          _ny,
                                          _xmin,
                                          _xmax,
                                          _ymin,
                                          _ymax,
                                          elem_type,
                                          _gauss_lobatto_grid);
      break;
    case 3:
      MeshTools::Generation::build_cube(static_cast<UnstructuredMesh &>(*mesh),
                                        _nx,
                                        _ny,
                                        _nz,
                                        _xmin,
                                        _xmax,
                                        _ymin,
                                        _ymax,
                                        _zmin,
                                        _zmax,
                                        elem_type,
                                        _gauss_lobatto_grid);
      break;
  }

  if (_has_subdomain_ids)
  {
    auto & bids = getParam<std::vector<SubdomainID>>("subdomain_ids");
    if (bids.size() != _nx * _ny * _nz && bids.size() != 1)
      paramError("subdomain_ids",
                 "Size must equal to the product of number of elements in all directions, or one.");
    for (auto & elem : mesh->element_ptr_range())
    {
      const Point p = elem->vertex_average();
      unsigned int ix = std::floor((p(0) - _xmin) / (_xmax - _xmin) * _nx);
      unsigned int iy = std::floor((p(1) - _ymin) / (_ymax - _ymin) * _ny);
      unsigned int iz = std::floor((p(2) - _zmin) / (_zmax - _zmin) * _nz);
      unsigned int i = iz * _nx * _ny + iy * _nx + ix;
      if (bids.size() == 1)
        elem->subdomain_id() = bids[0];
      else
        elem->subdomain_id() = bids[i];
    }
  }

  if (isParamValid("subdomain_name"))
  {
    const auto & subdomain_name = getParam<SubdomainName>("subdomain_name");
    if (isParamValid("subdomain_ids"))
    {
      const auto & bids = getParam<std::vector<SubdomainID>>("subdomain_ids");
      if (bids.size() > 1)
        paramError(
            "subdomain_ids",
            "Specifying a subdomain_name is only supported for a single entry in subdomain_ids");
      else
        mesh->subdomain_name(bids[0]) = subdomain_name;
    }
    else
      mesh->subdomain_name(0) = subdomain_name;
  }

  // rename and shift boundaries
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Copy, since we're modifying the container mid-iteration
  const auto mesh_boundary_ids = boundary_info.get_global_boundary_ids();
  for (auto rit = mesh_boundary_ids.rbegin(); rit != mesh_boundary_ids.rend(); ++rit)
  {
    const std::string old_sideset_name = boundary_info.sideset_name(*rit);
    const std::string old_nodeset_name = boundary_info.nodeset_name(*rit);

    MeshTools::Modification::change_boundary_id(*mesh, *rit, *rit + _boundary_id_offset);

    boundary_info.sideset_name(*rit + _boundary_id_offset) =
        _boundary_name_prefix + old_sideset_name;
    boundary_info.nodeset_name(*rit + _boundary_id_offset) =
        _boundary_name_prefix + old_nodeset_name;
  }

  // Apply the bias if any exists
  if (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0)
  {
    const auto MIN = std::numeric_limits<Real>::max();

    // Biases
    std::array<Real, LIBMESH_DIM> bias = {
        {_bias_x, _dim > 1 ? _bias_y : 1.0, _dim > 2 ? _bias_z : 1.0}};

    // "width" of the mesh in each direction
    std::array<Real, LIBMESH_DIM> width = {
        {_xmax - _xmin, _dim > 1 ? _ymax - _ymin : 0, _dim > 2 ? _zmax - _zmin : 0}};

    // Min mesh extent in each direction.
    std::array<Real, LIBMESH_DIM> mins = {{_xmin, _dim > 1 ? _ymin : MIN, _dim > 2 ? _zmin : MIN}};

    // Number of elements in each direction.
    std::array<unsigned int, LIBMESH_DIM> nelem = {{_nx, _dim > 1 ? _ny : 1, _dim > 2 ? _nz : 1}};

    // We will need the biases raised to integer powers in each
    // direction, so let's pre-compute those...
    std::array<std::vector<Real>, LIBMESH_DIM> pows;
    for (unsigned int dir = 0; dir < LIBMESH_DIM; ++dir)
    {
      pows[dir].resize(nelem[dir] + 1);
      pows[dir][0] = 1.0;
      for (unsigned int i = 1; i < pows[dir].size(); ++i)
        pows[dir][i] = pows[dir][i - 1] * bias[dir];
    }

    // Loop over the nodes and move them to the desired location
    for (auto & node_ptr : mesh->node_ptr_range())
    {
      Node & node = *node_ptr;

      for (unsigned int dir = 0; dir < LIBMESH_DIM; ++dir)
      {
        if (width[dir] != 0. && bias[dir] != 1.)
        {
          // Compute the scaled "index" of the current point.  This
          // will either be close to a whole integer or a whole
          // integer+0.5 for quadratic nodes.
          Real float_index = (node(dir) - mins[dir]) * nelem[dir] / width[dir];

          Real integer_part = 0;
          Real fractional_part = std::modf(float_index, &integer_part);

          // Figure out where to move the node...
          if (std::abs(fractional_part) < TOLERANCE || std::abs(fractional_part - 1.0) < TOLERANCE)
          {
            // If the fractional_part ~ 0.0 or 1.0, this is a vertex node, so
            // we don't need an average.
            //
            // Compute the "index" we are at in the current direction.  We
            // round to the nearest integral value to get this instead
            // of using "integer_part", since that could be off by a
            // lot (e.g. we want 3.9999 to map to 4.0 instead of 3.0).
            int index = round(float_index);

            mooseAssert(index >= static_cast<int>(0) && index < static_cast<int>(pows[dir].size()),
                        "Scaled \"index\" out of range");

            // Move node to biased location.
            node(dir) =
                mins[dir] + width[dir] * (1. - pows[dir][index]) / (1. - pows[dir][nelem[dir]]);
          }
          else if (std::abs(fractional_part - 0.5) < TOLERANCE)
          {
            // If the fractional_part ~ 0.5, this is a midedge/face
            // (i.e. quadratic) node.  We don't move those with the same
            // bias as the vertices, instead we put them midway between
            // their respective vertices.
            //
            // Also, since the fractional part is nearly 0.5, we know that
            // the integer_part will be the index of the vertex to the
            // left, and integer_part+1 will be the index of the
            // vertex to the right.
            node(dir) = mins[dir] +
                        width[dir] *
                            (1. - 0.5 * (pows[dir][integer_part] + pows[dir][integer_part + 1])) /
                            (1. - pows[dir][nelem[dir]]);
          }
          else
          {
            // We don't yet handle anything higher order than quadratic...
            mooseError("Unable to bias node at node(", dir, ")=", node(dir));
          }
        }
      }
    }
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
