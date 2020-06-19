//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ParsedAddSideset.h"
#include "Conversion.h"
#include "MooseMesh.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

registerMooseObjectReplaced("MooseApp",
                            ParsedAddSideset,
                            "11/30/2019 00:00",
                            ParsedGenerateSideset);

template <>
InputParameters
validParams<ParsedAddSideset>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params += FunctionParserUtils<false>::validParams();
  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<BoundaryName>("new_sideset_name", "The name of the new sideset");
  params.addParam<std::vector<SubdomainID>>(
      "included_subdomain_ids",
      "A set of subdomain ids whose sides will be included in the new sidesets");
  params.addParam<Point>(
      "normal",
      Point(),
      "If provided specifies the normal vector on sides that are added to the new ");
  params.addParam<std::vector<std::string>>(
      "constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addClassDescription("A MeshModifier that adds element's sides to a sideset if the "
                             "centroid satisfies the combinatorial_geometry expression, (and "
                             "optionally) "
                             "if one of the side's elements is in included_subdomain_ids and if it "
                             "features the correct normal.");
  return params;
}

ParsedAddSideset::ParsedAddSideset(const InputParameters & parameters)
  : AddSideSetsBase(parameters),
    FunctionParserUtils<false>(parameters),
    _function(parameters.get<std::string>("combinatorial_geometry")),
    _sideset_name(getParam<BoundaryName>("new_sideset_name")),
    _check_subdomains(isParamValid("included_subdomain_ids")),
    _check_normal(parameters.isParamSetByUser("normal")),
    _included_ids(_check_subdomains
                      ? parameters.get<std::vector<SubdomainID>>("included_subdomain_ids")
                      : std::vector<SubdomainID>()),
    _normal(getParam<Point>("normal"))
{
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

void
ParsedAddSideset::modify()
{
  // this mesh modifier works only on replicated meshes
  _mesh_ptr->errorIfDistributedMesh("ParsedAddSideset");

  setup();

  MeshBase & mesh = _mesh_ptr->getMesh();

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs({_sideset_name}, true);
  mooseAssert(boundary_ids.size() == 1, "Length of boundary_ids should be one");

  for (const auto & elem : mesh.active_element_ptr_range())
  {
    SubdomainID curr_subdomain = elem->subdomain_id();

    // check if the element is included
    if (_check_subdomains &&
        std::find(_included_ids.begin(), _included_ids.end(), curr_subdomain) ==
            _included_ids.end())
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      const std::vector<Point> & normals = _fe_face->get_normals();
      _fe_face->reinit(elem, side);

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
}
