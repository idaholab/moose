//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetExtruderGenerator.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", SideSetExtruderGenerator);

InputParameters
SideSetExtruderGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Takes a 1D or 2D mesh and extrudes the entire structure along the "
                             "specified axis increasing the dimensionality of the mesh.");

  // list params
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<RealVectorValue>("extrusion_vector",
                                           "The direction and length of the extrusion");
  params.addParam<unsigned int>("num_layers", 1, "The number of layers in the extruded mesh");
  params.addRequiredParam<BoundaryName>("sideset",
                                        "The side set (boundary) that will be extruded from");

  return params;
}

SideSetExtruderGenerator::SideSetExtruderGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _original_input(getParam<MeshGeneratorName>("input")),
    _extrusion_vector(getParam<RealVectorValue>("extrusion_vector")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _sideset_name(getParam<BoundaryName>("sideset"))
{
  // constants needed only temporarily
  const BoundaryName _EXTRUDED_BLOCK_NAME = "extruded_block";
  const BoundaryName _SIDESET_TO_BE_STITCHED = "to_be_stitched";

  // sub generators
  {
    auto params = _app.getFactory().getValidParams("LowerDBlockFromSidesetGenerator");

    params.set<MeshGeneratorName>("input") = _original_input;
    params.set<SubdomainName>("new_block_name") = _EXTRUDED_BLOCK_NAME;
    params.set<std::vector<BoundaryName>>("sidesets") = {_sideset_name};

    // generate lower dimensional mesh from the given sidesets
    _build_mesh = &addMeshSubgenerator(
        "LowerDBlockFromSidesetGenerator", name() + "_lowerDgeneration", params);
  }

  {
    auto params = _app.getFactory().getValidParams("BlockToMeshConverterGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_lowerDgeneration";
    params.set<std::vector<SubdomainName>>("target_blocks") = {_EXTRUDED_BLOCK_NAME};

    // convert lower dimensional block to a separate mesh
    _build_mesh =
        &addMeshSubgenerator("BlockToMeshConverterGenerator", name() + "_blockToMesh", params);
  }

  {
    auto params = _app.getFactory().getValidParams("MeshExtruderGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_blockToMesh";
    params.set<RealVectorValue>("extrusion_vector") = _extrusion_vector;
    params.set<unsigned int>("num_layers") = _num_layers;
    params.set<std::vector<BoundaryName>>("bottom_sideset") = {_SIDESET_TO_BE_STITCHED};

    // extrude the new, separate mesh into a higher dimension
    _build_mesh = &addMeshSubgenerator("MeshExtruderGenerator", name() + "_extruder", params);
  }

  {
    auto params = _app.getFactory().getValidParams("StitchedMeshGenerator");

    // order of vector elements matters for this generator
    // here order by: original mesh first, our custom mesh second
    params.set<std::vector<MeshGeneratorName>>("inputs") = {_original_input, name() + "_extruder"};

    params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
        {_sideset_name, _SIDESET_TO_BE_STITCHED}};

    // stitch the newly made high-dimensional mesh back to the original mesh
    _build_mesh = &addMeshSubgenerator("StitchedMeshGenerator", name() + "_stitched", params);
  }
}

std::unique_ptr<MeshBase>
SideSetExtruderGenerator::generate()
{
  return dynamic_pointer_cast<MeshBase>(*_build_mesh);
}
