//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedGenerateSideset.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

#include <typeinfo>

registerMooseObject("MooseApp", ParsedGenerateSideset);

defineLegacyParams(ParsedGenerateSideset);

InputParameters
ParsedGenerateSideset::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<BoundaryName>("new_sideset_name", "The name of the new sideset");
  params.addParam<std::vector<subdomain_id_type>>(
      "included_subdomain_ids",
      "A set of subdomain ids whose sides will be included in the new sidesets");
  params.addParam<std::vector<subdomain_id_type>>(
      "included_neighbor_ids",
      "A set of neighboring subdomain ids. A face is only added if the subdomain id of the "
      "neighbor is in this set");
  params.addParam<Point>(
      "normal",
      Point(),
      "If provided specifies the normal vector on sides that are added to the new ");
  params.addParam<std::vector<std::string>>(
      "constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addClassDescription("A MeshModifier that adds element sides to a sideset if the "
                             "centroid satisfies the `combinatorial_geometry` expression. "
                             "Optionally, element sides are also added if they are included in "
                             "`included_subdomain_ids` and if they feature the designated normal.");

  return params;
}

ParsedGenerateSideset::ParsedGenerateSideset(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _function(parameters.get<std::string>("combinatorial_geometry")),
    _sideset_name(getParam<BoundaryName>("new_sideset_name")),
    _check_subdomains(isParamValid("included_subdomain_ids")),
    _check_neighbor_subdomains(isParamValid("included_neighbor_ids")),
    _check_normal(parameters.isParamSetByUser("normal")),
    _included_ids(_check_subdomains
                      ? parameters.get<std::vector<SubdomainID>>("included_subdomain_ids")
                      : std::vector<SubdomainID>()),
    _included_neighbor_ids(_check_neighbor_subdomains
                               ? parameters.get<std::vector<SubdomainID>>("included_neighbor_ids")
                               : std::vector<SubdomainID>()),
    _normal(getParam<Point>("normal"))
{
  if (typeid(_input).name() == typeid(std::unique_ptr<DistributedMesh>).name())
    mooseError("GenerateAllSideSetsByNormals only works with ReplicatedMesh.");

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (_func_F->Parse(_function, "x,y,z") >= 0)
    mooseError("Invalid function\n",
               _function,
               "\nin ParsedAddSideset ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  _func_params.resize(3);
}

std::unique_ptr<MeshBase>
ParsedGenerateSideset::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  setup(*mesh);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Get the BoundaryIDs from the mesh
  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, {_sideset_name}, true);
  mooseAssert(boundary_ids.size() == 1, "Length of boundary_ids should be one");

  for (const auto & elem : mesh->active_element_ptr_range())
  {
    subdomain_id_type curr_subdomain = elem->subdomain_id();

    // check if the element is included
    if (_check_subdomains &&
        std::find(_included_ids.begin(), _included_ids.end(), curr_subdomain) ==
            _included_ids.end())
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      const std::vector<Point> & normals = _fe_face->get_normals();
      _fe_face->reinit(elem, side);

      // check if the neighboring elems subdomain is included
      if (_check_neighbor_subdomains)
      {
        const Elem * neighbor = elem->neighbor_ptr(side);
        // if the neighbor does not exist, then skip this face; we only add sidesets
        // between existing elems if _check_neighbor_subdomains is true
        if (!neighbor)
          continue;

        subdomain_id_type curr_neighbor_subdomain = neighbor->subdomain_id();
        if (std::find(_included_neighbor_ids.begin(),
                      _included_neighbor_ids.end(),
                      curr_neighbor_subdomain) == _included_neighbor_ids.end())
          continue;
      }

      // check normal if requested
      if (_check_normal && std::abs(1.0 - _normal * normals[0]) > _variance)
        continue;

      // check expression
      std::unique_ptr<Elem> curr_side = elem->side_ptr(side);
      _func_params[0] = curr_side->centroid()(0);
      _func_params[1] = curr_side->centroid()(1);
      _func_params[2] = curr_side->centroid()(2);
      if (evaluate(_func_F))
        boundary_info.add_side(elem, side, boundary_ids[0]);
    }
  }
  finalize();
  boundary_info.sideset_name(boundary_ids[0]) = _sideset_name;
  boundary_info.nodeset_name(boundary_ids[0]) = _sideset_name;

  return dynamic_pointer_cast<MeshBase>(mesh);
}
