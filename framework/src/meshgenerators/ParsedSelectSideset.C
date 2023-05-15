//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedSelectSideset.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/boundary_info.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

#include <typeinfo>

registerMooseObject("MooseApp", ParsedSelectSideset);

InputParameters
ParsedSelectSideset::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<BoundaryName>("new_boundary_name", "The name of the new sideset");
  params.addRequiredParam<BoundaryName>("old_boundary_name", "The name of the exisiting sideset");
  params.addParam<std::vector<std::string>>("constant_names",
                                            "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addClassDescription(
      "Defines a new sideset, which is the intersect of a exisiting sideset and"
      "the combinatorial geometry.");
  return params;
}

ParsedSelectSideset::ParsedSelectSideset(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _function(parameters.get<std::string>("combinatorial_geometry")),
    _new_boundary_name(getParam<BoundaryName>("new_boundary_name")),
    _old_boundary_name(getParam<BoundaryName>("old_boundary_name"))
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

std::unique_ptr<MeshBase>
ParsedSelectSideset::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseWarning(
        "ParsedSelectSideset is not implemented for distributed meshes. Make sure the "
        "parsed sideset does NOT cross any mesh distribution boundaries, using the ProcessorAux");

  setup(*mesh);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Get the old boundary id from the boundary name
  auto old_boundary_id = MooseMeshUtils::getBoundaryID(_old_boundary_name, *mesh);

  // check that old boundary id/name exist in the mesh
  if (old_boundary_id == Moose::INVALID_BOUNDARY_ID)
    paramError(
        "boundaries", "The boundary '", _old_boundary_name, "' was not found within the mesh");

  // make new boundary id
  auto new_boundary_id = MooseMeshUtils::getNextFreeBoundaryID(*mesh);

  // loop over element
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    // get sides of the old boundary if contained in current element
    auto old_boundary_sides = boundary_info.sides_with_boundary_id(elem, old_boundary_id);

    // loop over sides of old boundary
    for (const auto & side : old_boundary_sides)
    {
      // check expression
      std::unique_ptr<Elem> curr_side = elem->side_ptr(side);
      _func_params[0] = curr_side->vertex_average()(0);
      _func_params[1] = curr_side->vertex_average()(1);
      _func_params[2] = curr_side->vertex_average()(2);
      if (evaluate(_func_F))
        boundary_info.add_side(elem, side, new_boundary_id);
    }
  }
  finalize();

  // Write the name alias of the boundary id to the mesh boundary info
  boundary_info.sideset_name(new_boundary_id) = _new_boundary_name;
  boundary_info.nodeset_name(new_boundary_id) = _new_boundary_name;

  return dynamic_pointer_cast<MeshBase>(mesh);
}