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

  params.addClassDescription("Takes a 1D or 2D mesh and extrudes a selected sideset along the "
                             "specified axis.");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<RealVectorValue>("extrusion_vector",
                                           "The direction and length of the extrusion as a vector");
  params.addParam<unsigned int>("num_layers", 1, "The number of layers in the extruded mesh");
  params.addRequiredParam<BoundaryName>("sideset", "The sideset (boundary) that will be extruded");

  return params;
}

SideSetExtruderGenerator::SideSetExtruderGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _original_input(getParam<MeshGeneratorName>("input")),
    _extrusion_vector(getParam<RealVectorValue>("extrusion_vector")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _sideset_name(getParam<BoundaryName>("sideset"))
{
  // The input "input" is used by the first sub generator; this allows us to declare
  // that this dependency is not a dependency of SideSetExtruderGenerator but instead
  // the LowerDBlockFromSidesetGenerator below
  declareMeshForSub("input");

  const SubdomainName extruded_block_name = "extruded_block_" + name();
  const BoundaryName sideset_to_stitch = "to_be_stitched_" + name();

  // sub generators
  {
    auto params = _app.getFactory().getValidParams("LowerDBlockFromSidesetGenerator");

    params.set<MeshGeneratorName>("input") = _original_input;
    params.set<SubdomainName>("new_block_name") = extruded_block_name;
    params.set<std::vector<BoundaryName>>("sidesets") = {_sideset_name};

    // generate lower dimensional mesh from the given sideset
    addMeshSubgenerator("LowerDBlockFromSidesetGenerator", name() + "_lowerDgeneration", params);
  }

  {
    auto params = _app.getFactory().getValidParams("BlockToMeshConverterGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_lowerDgeneration";
    params.set<std::vector<SubdomainName>>("target_blocks") = {extruded_block_name};

    // convert lower dimensional block to a separate mesh
    addMeshSubgenerator("BlockToMeshConverterGenerator", name() + "_blockToMesh", params);
  }

  {
    auto params = _app.getFactory().getValidParams("MeshExtruderGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_blockToMesh";
    params.set<RealVectorValue>("extrusion_vector") = _extrusion_vector;
    params.set<unsigned int>("num_layers") = _num_layers;
    params.set<std::vector<BoundaryName>>("bottom_sideset") = {sideset_to_stitch};

    // extrude the new, separate mesh into a higher dimension
    addMeshSubgenerator("MeshExtruderGenerator", name() + "_extruder", params);
  }

  {
    auto params = _app.getFactory().getValidParams("StitchedMeshGenerator");

    // order of vector elements matters for this generator
    // here order by: original mesh first, our custom mesh second
    params.set<std::vector<MeshGeneratorName>>("inputs") = {_original_input, name() + "_extruder"};

    params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
        {_sideset_name, sideset_to_stitch}};

    // stitch the newly made high-dimensional mesh back to the original mesh
    addMeshSubgenerator("StitchedMeshGenerator", name() + "_stitched", params);
    _build_mesh = &getMeshByName(name() + "_stitched");
  }
}

std::unique_ptr<MeshBase>
SideSetExtruderGenerator::generate()
{
  return std::move(*_build_mesh);
}
