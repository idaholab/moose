//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinerGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "DelimitedFileReader.h"
#include "MooseMeshUtils.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/point.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", CombinerGenerator);

InputParameters
CombinerGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Combine multiple meshes (or copies of one mesh) together into one (disjoint) mesh.  Can "
      "optionally translate those meshes before combining them.");

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The input MeshGenerators.  This can either be N generators or 1 generator.  If only 1 is "
      "given then 'positions' must also be given.");

  params.addParam<std::vector<Point>>(
      "positions",
      "The (optional) position of each given mesh.  If N 'inputs' were given then this must either "
      "be left blank or N positions must be given.  If 1 input was given then this MUST be "
      "provided.");

  params.addParam<std::vector<FileName>>(
      "positions_file", "Alternative way to provide the position of each given mesh.");

  params.addParam<bool>("avoid_merging_subdomains",
                        false,
                        "Whether to prevent merging subdomains by offsetting ids. The first mesh "
                        "in the input will keep the same subdomains ids, the others will have "
                        "offsets. All subdomain names will remain valid");
  params.addParam<bool>("avoid_merging_boundaries",
                        false,
                        "Whether to prevent merging sidesets by offsetting ids. The first mesh "
                        "in the input will keep the same boundary ids, the others will have "
                        "offsets. All boundary names will remain valid");

  return params;
}

CombinerGenerator::CombinerGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _meshes(getMeshes("inputs")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _avoid_merging_subdomains(getParam<bool>("avoid_merging_subdomains")),
    _avoid_merging_boundaries(getParam<bool>("avoid_merging_boundaries"))
{
  if (_input_names.empty())
    paramError("input_names", "You need to specify at least one MeshGenerator as an input.");

  if (isParamValid("positions") && isParamValid("positions_file"))
    mooseError("Both 'positions' and 'positions_file' cannot be specified simultaneously in "
               "CombinerGenerator ",
               _name);

  if (_input_names.size() == 1)
    if (!isParamValid("positions") && !isParamValid("positions_file"))
      paramError("positions",
                 "If only one input mesh is given, then 'positions' or 'positions_file' must also "
                 "be supplied");
}

void
CombinerGenerator::fillPositions()
{
  if (isParamValid("positions"))
  {
    _positions = getParam<std::vector<Point>>("positions");

    // the check in the constructor wont catch error where the user sets positions = ''
    if ((_input_names.size() == 1) && _positions.empty())
      paramError("positions", "If only one input mesh is given, then 'positions' cannot be empty.");

    if (_input_names.size() != 1)
      if (_positions.size() && (_input_names.size() != _positions.size()))
        paramError(
            "positions",
            "If more than one input mesh is provided then the number of positions provided must "
            "exactly match the number of input meshes.");
  }
  else if (isParamValid("positions_file"))
  {
    std::vector<FileName> positions_file = getParam<std::vector<FileName>>("positions_file");

    // the check in the constructor wont catch error where the user sets positions_file = ''
    if ((_input_names.size() == 1) && positions_file.empty())
      paramError("positions_file",
                 "If only one input mesh is given, then 'positions_file' cannot be empty.");

    for (const auto & f : positions_file)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<Point> & data = file.getDataAsPoints();

      if (_input_names.size() != 1)
        if (data.size() && (_input_names.size() != data.size()))
          paramError("positions_file",
                     "If more than one input mesh is provided then the number of positions must "
                     "exactly match the number of input meshes.");

      for (const auto & d : data)
        _positions.push_back(d);
    }
  }
}

std::unique_ptr<MeshBase>
CombinerGenerator::generate()
{
  // Two cases:
  // 1. Multiple input meshes and optional positions
  // 2. One input mesh and multiple positions
  fillPositions();

  // Case 1
  if (_meshes.size() != 1)
  {
    // merge all meshes into the first one
    auto mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[0]);

    if (!mesh)
      paramError("inputs", _input_names[0], " is not a valid unstructured mesh");

    // Move the first input mesh if applicable
    if (_positions.size())
    {
      MeshTools::Modification::translate(
          *mesh, _positions[0](0), _positions[0](1), _positions[0](2));
    }

    // Read in all of the other meshes
    for (MooseIndex(_meshes) i = 1; i < _meshes.size(); ++i)
    {
      auto other_mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[i]);

      if (!other_mesh)
        paramError("inputs", _input_names[i], " is not a valid unstructured mesh");

      // Move It
      if (_positions.size())
      {
        MeshTools::Modification::translate(
            *other_mesh, _positions[i](0), _positions[i](1), _positions[i](2));
      }

      MooseMeshUtils::copyIntoMesh(*this,
                                   *mesh,
                                   *other_mesh,
                                   _avoid_merging_subdomains,
                                   _avoid_merging_boundaries,
                                   _communicator);
    }

    mesh->unset_is_prepared();
    return dynamic_pointer_cast<MeshBase>(mesh);
  }
  else // Case 2
  {
    auto input_mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[0]);

    if (!input_mesh)
      paramError("inputs", _input_names[0], " is not a valid unstructured mesh");

    // Make a copy and displace it in order to get the final mesh started
    auto copy =
        input_mesh->clone(); // This is required because dynamic_pointer_cast() requires an l-value
    auto final_mesh = dynamic_pointer_cast<UnstructuredMesh>(copy);

    if (!final_mesh)
      mooseError("Unable to copy mesh!");

    MeshTools::Modification::translate(
        *final_mesh, _positions[0](0), _positions[0](1), _positions[0](2));

    // Here's the way this is going to work:
    // I'm going to make one more copy of the input_mesh so that I can move it and copy it in
    // Then, after it's copied in I'm going to reset its coordinates by looping over the input_mesh
    // and resetting the nodal positions.
    // This could be done without the copy - you would translate the mesh then translate it back...
    // However, I'm worried about floating point roundoff.  If you were doing this 100,000 times or
    // more then the mesh could "drift" away from its original position.  I really want the
    // translations to be exact each time.
    // I suppose that it is technically possible to just save off a datastructure (map, etc.) that
    // could hold the nodal positions only (instead of a copy of the mesh) but I'm not sure that
    // would really save much... we'll see if it shows up in profiling somewhere
    copy = input_mesh->clone();
    auto translated_mesh = dynamic_pointer_cast<UnstructuredMesh>(copy);

    if (!translated_mesh)
      mooseError("Unable to copy mesh!");

    for (MooseIndex(_meshes) i = 1; i < _positions.size(); ++i)
    {
      // Move
      MeshTools::Modification::translate(
          *translated_mesh, _positions[i](0), _positions[i](1), _positions[i](2));

      // Copy into final mesh
      MooseMeshUtils::copyIntoMesh(*this,
                                   *final_mesh,
                                   *translated_mesh,
                                   _avoid_merging_subdomains,
                                   _avoid_merging_boundaries,
                                   _communicator);

      // Reset nodal coordinates
      for (auto translated_node_ptr : translated_mesh->node_ptr_range())
      {
        auto & translated_node = *translated_node_ptr;
        auto & input_node = input_mesh->node_ref(translated_node_ptr->id());

        for (MooseIndex(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; i++)
          translated_node(i) = input_node(i);
      }
    }

    final_mesh->unset_is_prepared();
    return dynamic_pointer_cast<MeshBase>(final_mesh);
  }
}
