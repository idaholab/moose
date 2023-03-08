//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FillBetweenCurvesGenerator.h"
#include "FillBetweenPointVectorsTools.h"

#include "CastUniquePointer.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", FillBetweenCurvesGenerator);

InputParameters
FillBetweenCurvesGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input_mesh_1",
                                             "The input mesh that contains curve 1");
  params.addRequiredParam<MeshGeneratorName>("input_mesh_2",
                                             "The input mesh that contains curve 1");
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
  params.addClassDescription("This FillBetweenCurvesGenerator object is designed to generate a "
                             "transition layer to connect two boundaries of two input meshes.");
  return params;
}

FillBetweenCurvesGenerator::FillBetweenCurvesGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_name_1(getParam<MeshGeneratorName>("input_mesh_1")),
    _input_name_2(getParam<MeshGeneratorName>("input_mesh_2")),
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
    _input_1(getMeshByName(_input_name_1)),
    _input_2(getMeshByName(_input_name_2))
{
  if (_input_name_1.compare(_input_name_2) == 0)
    paramError("input_mesh_2", "This parameter must be different from input_mesh_1.");
}

std::unique_ptr<MeshBase>
FillBetweenCurvesGenerator::generate()
{
  auto input_mesh_1 = dynamic_pointer_cast<ReplicatedMesh>(std::move(_input_1));
  auto input_mesh_2 = dynamic_pointer_cast<ReplicatedMesh>(std::move(_input_2));
  if (!input_mesh_1)
    paramError("input_mesh_1", "Input is not a replicated mesh, which is required.");
  if (!input_mesh_2)
    paramError("input_mesh_2", "Input is not a replicated mesh, which is required.");

  MeshTools::Modification::translate(
      *input_mesh_1, _mesh_1_shift(0), _mesh_1_shift(1), _mesh_1_shift(2));
  MeshTools::Modification::translate(
      *input_mesh_2, _mesh_2_shift(0), _mesh_2_shift(1), _mesh_2_shift(2));

  Real max_input_mesh_1_node_radius;
  Real max_input_mesh_2_node_radius;
  std::vector<dof_id_type> curve_1_ordered_nodes;
  std::vector<dof_id_type> curve_2_ordered_nodes;

  try
  {
    FillBetweenPointVectorsTools::isCurveOpenSingleSegment(*input_mesh_1,
                                                           max_input_mesh_1_node_radius,
                                                           curve_1_ordered_nodes,
                                                           curveCentroidPoint(*input_mesh_1));
  }
  catch (MooseException & e)
  {
    paramError("curve_1", e.what());
  }
  try
  {
    FillBetweenPointVectorsTools::isCurveOpenSingleSegment(*input_mesh_2,
                                                           max_input_mesh_2_node_radius,
                                                           curve_2_ordered_nodes,
                                                           curveCentroidPoint(*input_mesh_2));
  }
  catch (MooseException & e)
  {
    paramError("curve_2", e.what());
  }

  std::vector<Point> positions_vector_1;
  std::vector<Point> positions_vector_2;

  for (auto & curve_1_node_id : curve_1_ordered_nodes)
    positions_vector_1.push_back(*input_mesh_1->node_ptr(curve_1_node_id));

  for (auto & curve_2_node_id : curve_2_ordered_nodes)
    positions_vector_2.push_back(*input_mesh_2->node_ptr(curve_2_node_id));

  const boundary_id_type input_boundary_1_id = _input_boundary_1_id;
  const boundary_id_type input_boundary_2_id = _input_boundary_2_id;
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

  return dynamic_pointer_cast<MeshBase>(mesh);
}

Point
FillBetweenCurvesGenerator::curveCentroidPoint(const ReplicatedMesh & curve)
{
  Point pt_tmp = Point(0.0, 0.0, 0.0);
  Real length_tmp = 0.0;
  for (const auto elem : curve.element_ptr_range())
  {
    Real elem_length = elem->hmax();
    pt_tmp += (elem->vertex_average()) * elem_length;
    length_tmp += elem_length;
  }
  return pt_tmp / length_tmp;
}
