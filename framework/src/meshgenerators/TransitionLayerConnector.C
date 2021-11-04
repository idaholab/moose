//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransitionLayerConnector.h"
#include "TransitionLayerTools.h"

#include "MooseMeshUtils.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", TransitionLayerConnector);

InputParameters
TransitionLayerConnector::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input_mesh_1",
                                             "The input mesh that contains boundary_1");
  params.addRequiredParam<MeshGeneratorName>("input_mesh_2",
                                             "The input mesh that contains boundary_2");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary_1", "the first boundary that need to be connected.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary_2", "the second boundary that need to be connected.");
  params.addParam<Point>(
      "mesh_1_shift", Point(0.0, 0.0, 0.0), "The translate vector to be applied to input_mesh_1");
  params.addParam<Point>(
      "mesh_2_shift", Point(0.0, 0.0, 0.0), "The translate vector to be applied to input_mesh_2");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_layers", "num_layers>0", "Layers of elements for transition.");
  params.addParam<subdomain_id_type>("block_id", 1, "ID to be assigned to the block.");
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
      "parameter is not used if other biasing option is selected.");
  params.addParam<bool>(
      "keep_inputs",
      false,
      "Whether to output the input meshes stitched with the transition layer connector.");
  params.addClassDescription("This TransitionLayerConnector object is designed to generate a "
                             "transition layer to connect two boundaries of two input meshes.");
  return params;
}

TransitionLayerConnector::TransitionLayerConnector(const InputParameters & parameters)
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
}

std::unique_ptr<MeshBase>
TransitionLayerConnector::generate()
{
  auto input_mesh_1 = dynamic_cast<ReplicatedMesh *>(_input_1.get());
  auto input_mesh_2 = dynamic_cast<ReplicatedMesh *>(_input_2.get());
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

  _input_mesh_1_external_bids = MooseMeshUtils::getBoundaryIDs(*input_mesh_1, _boundary_1, false);
  _input_mesh_2_external_bids = MooseMeshUtils::getBoundaryIDs(*input_mesh_2, _boundary_2, false);

  for (unsigned int i = 1; i < _input_mesh_1_external_bids.size(); i++)
  {
    MooseMeshUtils::changeBoundaryId(
        *input_mesh_1, _input_mesh_1_external_bids[i], _input_mesh_1_external_bids.front(), true);
  }
  for (unsigned int i = 1; i < _input_mesh_2_external_bids.size(); i++)
  {
    MooseMeshUtils::changeBoundaryId(
        *input_mesh_2, _input_mesh_2_external_bids[i], _input_mesh_2_external_bids.front(), true);
  }

  if (!isExternalBoundary(*input_mesh_1, _input_mesh_1_external_bids.front()))
    paramError("boundary_1", "The boundary provided is not an external boundary.");
  if (!isExternalBoundary(*input_mesh_2, _input_mesh_2_external_bids.front()))
    paramError("boundary_2", "The boundary provided is not an external boundary.");

  Real max_input_mesh_1_node_radius;
  unsigned short invalid_boundary_type_1;
  Real max_input_mesh_2_node_radius;
  unsigned short invalid_boundary_type_2;
  std::vector<dof_id_type> boundary_1_ordered_nodes;
  std::vector<dof_id_type> boundary_2_ordered_nodes;

  isBoundaryValid(*input_mesh_1,
                  max_input_mesh_1_node_radius,
                  invalid_boundary_type_1,
                  boundary_1_ordered_nodes,
                  _mesh_1_shift,
                  _input_mesh_1_external_bids.front());
  isBoundaryValid(*input_mesh_2,
                  max_input_mesh_2_node_radius,
                  invalid_boundary_type_2,
                  boundary_2_ordered_nodes,
                  _mesh_2_shift,
                  _input_mesh_2_external_bids.front());

  if (invalid_boundary_type_1 != 2)
    paramError("boundary_1", "The provided boundary is not a single-segment boundary.");
  if (invalid_boundary_type_2 != 2)
    paramError("boundary_2", "The provided boundary is not a single-segment boundary.");

  std::vector<Point> positions_vector_1;
  std::vector<Point> positions_vector_2;

  for (auto & boundary_1_node_id : boundary_1_ordered_nodes)
    positions_vector_1.push_back(*input_mesh_1->node_ptr(boundary_1_node_id));

  for (auto & boundary_2_node_id : boundary_2_ordered_nodes)
    positions_vector_2.push_back(*input_mesh_2->node_ptr(boundary_2_node_id));

  auto mesh = buildReplicatedMesh(2);
  TransitionLayerTools::transitionLayerGenerator(*mesh,
                                                 positions_vector_2,
                                                 positions_vector_1,
                                                 _num_layers,
                                                 _block_id,
                                                 _input_boundary_1_id,
                                                 _input_boundary_2_id,
                                                 _begin_side_boundary_id,
                                                 _end_side_boundary_id,
                                                 _type,
                                                 _name,
                                                 _use_quad_elements,
                                                 _bias_parameter,
                                                 _sigma);

  if (_keep_inputs)
  {
    mesh->stitch_meshes(
        *input_mesh_1, _input_boundary_1_id, _input_mesh_1_external_bids.front(), TOLERANCE, true);
    mesh->stitch_meshes(
        *input_mesh_2, _input_boundary_2_id, _input_mesh_2_external_bids.front(), TOLERANCE, true);
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

bool
TransitionLayerConnector::isBoundaryValid(ReplicatedMesh & mesh,
                                          Real & max_node_radius,
                                          unsigned short & invalid_type,
                                          std::vector<dof_id_type> & boundary_ordered_node_list,
                                          const Point origin_pt,
                                          const boundary_id_type bid) const
{
  max_node_radius = 0.0;
  invalid_type = 0;
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto side_list_tmp = boundary_info.build_side_list();
  unsigned int elem_counter = 0;
  std::vector<std::pair<dof_id_type, dof_id_type>> boundary_node_assm;
  bool isFlipped = false;
  for (unsigned int i = 0; i < side_list_tmp.size(); i++)
  {
    if (std::get<2>(side_list_tmp[i]) == bid)
    {
      elem_counter++;
      // store two nodes of each side
      boundary_node_assm.push_back(std::make_pair(mesh.elem_ptr(std::get<0>(side_list_tmp[i]))
                                                      ->side_ptr(std::get<1>(side_list_tmp[i]))
                                                      ->node_id(0),
                                                  mesh.elem_ptr(std::get<0>(side_list_tmp[i]))
                                                      ->side_ptr(std::get<1>(side_list_tmp[i]))
                                                      ->node_id(1)));
    }
  }
  // Start from the first element, try to find a chain of nodes
  boundary_ordered_node_list.push_back(boundary_node_assm.front().first);
  boundary_ordered_node_list.push_back(boundary_node_assm.front().second);
  // Remove the element that has been added to boundary_ordered_node_list
  boundary_node_assm.erase(boundary_node_assm.begin());
  const unsigned int boundary_node_assm_size_0 = boundary_node_assm.size();
  for (unsigned int i = 0; i < boundary_node_assm_size_0; i++)
  {
    // Find nodes to expand the chain
    dof_id_type end_node_id = boundary_ordered_node_list.back();
    auto isMatch1 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.first == end_node_id; };
    auto isMatch2 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.second == end_node_id; };
    auto result = std::find_if(boundary_node_assm.begin(), boundary_node_assm.end(), isMatch1);
    bool match_first;
    if (result == boundary_node_assm.end())
    {
      match_first = false;
      result = std::find_if(boundary_node_assm.begin(), boundary_node_assm.end(), isMatch2);
    }
    else
    {
      match_first = true;
    }
    // If found, add the node to boundary_ordered_node_list
    if (result != boundary_node_assm.end())
    {
      boundary_ordered_node_list.push_back(match_first ? (*result).second : (*result).first);
      boundary_node_assm.erase(result);
    }
    // If there are still elements in boundary_node_assm and result ==
    // boundary_node_assm.end(), this means the boundary is not a loop, the
    // boundary_ordered_node_list is flipped and try the other direction that has not
    // been examined yet.
    else
    {
      if (isFlipped)
      {
        // Flipped twice; this means the boundary has at least two segments.
        // This is invalid type #1
        invalid_type = 1;
        return false;
      }
      // mark the first flip event.
      isFlipped = true;
      std::reverse(boundary_ordered_node_list.begin(), boundary_ordered_node_list.end());
      // As this iteration is wasted, set the iterator backward
      i--;
    }
  }
  // If the code ever gets here, boundary_node_assm is empty.
  // If the boundary_ordered_node_list front and back are not the same, the boundary is not a loop.
  // This is not done inside the loop just for some potential applications in the future.
  if (boundary_ordered_node_list.front() != boundary_ordered_node_list.back())
  {
    // This is invalid type #2
    invalid_type = 2;
    return false;
  }
  // It the boundary is a loop, check if azimuthal angles change monotonically
  else
  {
    // Utilize cross product here.
    // If azimuthal angles change monotonically,
    // the z components of the cross products are always negative or positive.
    std::vector<Real> ordered_node_azi_list;
    for (unsigned int i = 0; i < boundary_ordered_node_list.size() - 1; i++)
    {
      ordered_node_azi_list.push_back(
          (*mesh.node_ptr(boundary_ordered_node_list[i]) - origin_pt)
              .cross(*mesh.node_ptr(boundary_ordered_node_list[i + 1]) - origin_pt)(2));
      // Use this opportunity to calculate maximum radius
      max_node_radius = std::max((*mesh.node_ptr(boundary_ordered_node_list[i]) - origin_pt).norm(),
                                 max_node_radius);
    }
    std::sort(ordered_node_azi_list.begin(), ordered_node_azi_list.end());
    if (ordered_node_azi_list.front() * ordered_node_azi_list.back() < 0.0)
    {
      // This is invalid type #3
      invalid_type = 3;
      return false;
    }
    else
      return true;
  }
}

bool
TransitionLayerConnector::isExternalBoundary(ReplicatedMesh & mesh,
                                             const boundary_id_type bid) const
{
  if (!mesh.is_prepared())
    mesh.find_neighbors();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto side_list = boundary_info.build_side_list();
  for (unsigned int i = 0; i < side_list.size(); i++)
  {
    if (std::get<2>(side_list[i]) == bid)
      if (mesh.elem_ptr(std::get<0>(side_list[i]))->neighbor_ptr(std::get<1>(side_list[i])) !=
          nullptr)
        return false;
  }
  return true;
}
