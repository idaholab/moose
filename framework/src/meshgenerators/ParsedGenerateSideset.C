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

InputParameters
ParsedGenerateSideset::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<BoundaryName>("new_sideset_name", "The name of the new sideset");

  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  // This sideset generator can only handle a single new sideset name, not a vector of names
  params.suppressParameter<std::vector<BoundaryName>>("new_boundary");

  params.addClassDescription(
      "A MeshGenerator that adds element sides to a sideset if the centroid of the side satisfies "
      "the `combinatorial_geometry` expression.");

  return params;
}

ParsedGenerateSideset::ParsedGenerateSideset(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    FunctionParserUtils<false>(parameters),
    _function(parameters.get<std::string>("combinatorial_geometry"))
{
  _boundary_names.push_back(getParam<BoundaryName>("new_sideset_name"));

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
    paramError(
        "combinatorial_geometry", "Invalid function\n", _function, "\n", _func_F->ErrorMsg());

  _func_params.resize(3);
}

std::unique_ptr<MeshBase>
ParsedGenerateSideset::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseWarning(
        "ParsedGenerateSideset is not implemented for distributed meshes. Make sure the "
        "parsed sideset does NOT cross any mesh distribution boundaries, using the ProcessorAux");

  setup(*mesh);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Get the BoundaryIDs from the mesh
  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true);
  mooseAssert(boundary_ids.size() == 1, "Length of boundary_ids should be one");

  for (const auto & elem : mesh->active_element_ptr_range())
  {
    // check if the element is included
    if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
      continue;

    for (const auto side : make_range(elem->n_sides()))
    {
      _fe_face->reinit(elem, side);
      // We'll just use the normal of the first qp
      const Point & face_normal = _fe_face->get_normals()[0];

      if (!elemSideSatisfiesRequirements(elem, side, *mesh, _normal, face_normal))
        continue;

      // check expression
      std::unique_ptr<Elem> curr_side = elem->side_ptr(side);
      _func_params[0] = curr_side->vertex_average()(0);
      _func_params[1] = curr_side->vertex_average()(1);
      _func_params[2] = curr_side->vertex_average()(2);
      if (evaluate(_func_F))
      {
        if (_replace)
          boundary_info.remove_side(elem, side);
        boundary_info.add_side(elem, side, boundary_ids[0]);
      }
    }
  }
  finalize();
  boundary_info.sideset_name(boundary_ids[0]) = _boundary_names[0];

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
