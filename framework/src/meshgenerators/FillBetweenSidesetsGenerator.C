//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FillBetweenSidesetsGenerator.h"
#include "FillBetweenPointVectorsTools.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", FillBetweenSidesetsGenerator);

InputParameters
FillBetweenSidesetsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input_mesh_1",
                                             "The input mesh that contains boundary_1");
  params.addRequiredParam<MeshGeneratorName>("input_mesh_2",
                                             "The input mesh that contains boundary_2");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary_1", "the first boundary that needs to be connected.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary_2", "the second boundary that needs to be connected.");
  params.addParam<Point>(
      "mesh_1_shift", Point(0.0, 0.0, 0.0), "Translation vector to be applied to input_mesh_1");
  params.addParam<Point>(
      "mesh_2_shift", Point(0.0, 0.0, 0.0), "Translation vector to be applied to input_mesh_2");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_layers", "num_layers>0", "Number of layers of elements created between the boundaries.");
  params.addParam<subdomain_id_type>("block_id", 1, "ID to be assigned to the transition layer.");
  params.addParam<boundary_id_type>(
      "input_boundary_1_id",
      10000,
      "Boundary ID to be assigned to the boundary defined by positions_vector_1.");
  params.addParam<boundary_id_type>(
      "input_boundary_2_id",
      10000,
      "Boundary ID to be assigned to the boundary defined by positions_vector_2.");
  params.addParam<boundary_id_type>("begin_side_boundary_id",
                                    10000,
                                    "Boundary ID to be assigned to the boundary connecting "
                                    "starting points of the positions_vectors.");
  params.addParam<boundary_id_type>("end_side_boundary_id",
                                    10000,
                                    "Boundary ID to be assigned to the boundary connecting ending "
                                    "points of the positions_vectors.");
  params.addParam<bool>(
      "use_quad_elements",
      false,
      "Whether QUAD4 instead of TRI3 elements are used to construct the transition layer.");
  params.addRangeCheckedParam<Real>(
      "bias_parameter",
      1.0,
      "bias_parameter>=0",
      "Parameter used to set up biasing of the layers: bias_parameter > 0.0 is used as the biasing "
      "factor; bias_parameter = 0.0 activates automatic biasing based on local node density on "
      "both input boundaries.");
  params.addRangeCheckedParam<Real>(
      "gaussian_sigma",
      3.0,
      "gaussian_sigma>0.0",
      "Gaussian parameter used to smoothen local node density for automatic biasing; this "
      "parameter is not used if another biasing option is selected.");
  params.addParam<bool>(
      "keep_inputs",
      true,
      "Whether to output the input meshes stitched with the transition layer connector.");
  params.addClassDescription("This FillBetweenSidesetsGenerator object is designed to generate a "
                             "transition layer to connect two boundaries of two input meshes.");
  return params;
}

FillBetweenSidesetsGenerator::FillBetweenSidesetsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_name_1(getParam<MeshGeneratorName>("input_mesh_1")),
    _input_name_2(getParam<MeshGeneratorName>("input_mesh_2")),
    _boundary_1(getParam<std::vector<BoundaryName>>("boundary_1")),
    _boundary_2(getParam<std::vector<BoundaryName>>("boundary_2")),
    _mesh_1_shift(getParam<Point>("mesh_1_shift")),
    _mesh_2_shift(getParam<Point>("mesh_2_shift")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _block_id(getParam<subdomain_id_type>("block_id")),
    _input_boundary_1_id(getParam<boundary_id_type>("input_boundary_1_id")),
    _input_boundary_2_id(getParam<boundary_id_type>("input_boundary_2_id")),
    _begin_side_boundary_id(getParam<boundary_id_type>("begin_side_boundary_id")),
    _end_side_boundary_id(getParam<boundary_id_type>("end_side_boundary_id")),
    _use_quad_elements(getParam<bool>("use_quad_elements")),
    _bias_parameter(getParam<Real>("bias_parameter")),
    _sigma(getParam<Real>("gaussian_sigma")),
    _keep_inputs(getParam<bool>("keep_inputs")),
    _input_1(getMeshByName(_input_name_1)),
    _input_2(getMeshByName(_input_name_2))
{
  if (_input_name_1.compare(_input_name_2) == 0)
    paramError("input_mesh_2", "This parameter must be different from input_mesh_1.");
}

std::unique_ptr<MeshBase>
FillBetweenSidesetsGenerator::generate()
{
  auto input_1 = std::move(_input_1);
  auto input_2 = std::move(_input_2);

  auto input_mesh_1 = dynamic_cast<ReplicatedMesh *>(input_1.get());
  auto input_mesh_2 = dynamic_cast<ReplicatedMesh *>(input_2.get());
  if (!input_mesh_1)
    paramError("input_mesh_1", "Input is not a replicated mesh, which is required.");
  if (*(input_mesh_1->elem_dimensions().begin()) != 2 ||
      *(input_mesh_1->elem_dimensions().rbegin()) != 2)
    paramError("input_mesh_1", "Only 2D meshes are supported.");
  if (!input_mesh_2)
    paramError("input_mesh_2", "Input is not a replicated mesh, which is required.");
  if (*(input_mesh_2->elem_dimensions().begin()) != 2 ||
      *(input_mesh_2->elem_dimensions().rbegin()) != 2)
    paramError("input_mesh_2", "Only 2D meshes are supported.");

  MeshTools::Modification::translate(
      *input_mesh_1, _mesh_1_shift(0), _mesh_1_shift(1), _mesh_1_shift(2));
  MeshTools::Modification::translate(
      *input_mesh_2, _mesh_2_shift(0), _mesh_2_shift(1), _mesh_2_shift(2));

  const auto input_mesh_1_external_bids =
      MooseMeshUtils::getBoundaryIDs(*input_mesh_1, _boundary_1, false);
  const auto input_mesh_2_external_bids =
      MooseMeshUtils::getBoundaryIDs(*input_mesh_2, _boundary_2, false);

  for (unsigned int i = 1; i < input_mesh_1_external_bids.size(); i++)
  {
    MooseMeshUtils::changeBoundaryId(
        *input_mesh_1, input_mesh_1_external_bids[i], input_mesh_1_external_bids.front(), true);
  }
  for (unsigned int i = 1; i < input_mesh_2_external_bids.size(); i++)
  {
    MooseMeshUtils::changeBoundaryId(
        *input_mesh_2, input_mesh_2_external_bids[i], input_mesh_2_external_bids.front(), true);
  }

  if (!FillBetweenPointVectorsTools::isExternalBoundary(*input_mesh_1,
                                                        input_mesh_1_external_bids.front()))
    paramError("boundary_1", "The boundary provided is not an external boundary.");
  if (!FillBetweenPointVectorsTools::isExternalBoundary(*input_mesh_2,
                                                        input_mesh_2_external_bids.front()))
    paramError("boundary_2", "The boundary provided is not an external boundary.");

  Real max_input_mesh_1_node_radius;
  Real max_input_mesh_2_node_radius;
  std::vector<dof_id_type> boundary_1_ordered_nodes;
  std::vector<dof_id_type> boundary_2_ordered_nodes;

  try
  {
    FillBetweenPointVectorsTools::isBoundaryOpenSingleSegment(
        *input_mesh_1,
        max_input_mesh_1_node_radius,
        boundary_1_ordered_nodes,
        MooseMeshUtils::meshCentroidCalculator(*input_mesh_1),
        input_mesh_1_external_bids.front());
  }
  catch (MooseException & e)
  {
    paramError("boundary_1", e.what());
  }
  try
  {
    FillBetweenPointVectorsTools::isBoundaryOpenSingleSegment(
        *input_mesh_2,
        max_input_mesh_2_node_radius,
        boundary_2_ordered_nodes,
        MooseMeshUtils::meshCentroidCalculator(*input_mesh_2),
        input_mesh_2_external_bids.front());
  }
  catch (MooseException & e)
  {
    paramError("boundary_2", e.what());
  }

  std::vector<Point> positions_vector_1;
  std::vector<Point> positions_vector_2;

  for (auto & boundary_1_node_id : boundary_1_ordered_nodes)
    positions_vector_1.push_back(*input_mesh_1->node_ptr(boundary_1_node_id));

  for (auto & boundary_2_node_id : boundary_2_ordered_nodes)
    positions_vector_2.push_back(*input_mesh_2->node_ptr(boundary_2_node_id));

  const boundary_id_type input_boundary_1_id = _keep_inputs ? (std::max({_input_boundary_1_id,
                                                                         _input_boundary_2_id,
                                                                         _begin_side_boundary_id,
                                                                         _end_side_boundary_id}) +
                                                               1)
                                                            : _input_boundary_1_id;
  const boundary_id_type input_boundary_2_id =
      _keep_inputs ? (input_boundary_1_id + 1) : _input_boundary_2_id;
  auto mesh = buildReplicatedMesh(2);
  FillBetweenPointVectorsTools::fillBetweenPointVectorsGenerator(*mesh,
                                                                 positions_vector_1,
                                                                 positions_vector_2,
                                                                 _num_layers,
                                                                 _block_id,
                                                                 input_boundary_1_id,
                                                                 input_boundary_2_id,
                                                                 _begin_side_boundary_id,
                                                                 _end_side_boundary_id,
                                                                 _type,
                                                                 _name,
                                                                 _use_quad_elements,
                                                                 _bias_parameter,
                                                                 _sigma);

  if (_keep_inputs)
  {
    mesh->prepare_for_use();
    mesh->stitch_meshes(*input_mesh_1,
                        input_boundary_1_id,
                        input_mesh_1_external_bids.front(),
                        TOLERANCE,
                        true,
                        false,
                        true,
                        true);
    mesh->stitch_meshes(*input_mesh_2,
                        input_boundary_2_id,
                        input_mesh_2_external_bids.front(),
                        TOLERANCE,
                        true,
                        false,
                        true,
                        true);
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
